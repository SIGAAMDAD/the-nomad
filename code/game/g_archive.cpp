// g_archive.cpp -- no game data archiving allowed within the vm to allow various mods and vanilla saves to work with each other

#include "g_game.h"
#include "g_archive.h"

#define NGD_MAGIC 0xff5ad1120
#define XOR_MAGIC 0xff

#define IDENT (('d'<<24)+('g'<<16)+('n'<<8)+'!')

/*

.ngd save file layout:

section    |  name   |  value  | type
----------------------------------------
HEADER     | ident   | !ngd    | int32
HEADER     | version | N/A     | uint64

*/

CGameArchive *g_pArchiveHandler;

enum {
	FT_CHAR,
	FT_SHORT,
	FT_INT,
	FT_LONG,

	FT_UCHAR,
	FT_USHORT,
	FT_UINT,
	FT_ULONG,

	FT_FLOAT,
	FT_VECTOR2,
	FT_VECTOR3,
	FT_VECTOR4,
	FT_STRING,

	FT_ARRAY
};

bool CGameArchive::ValidateHeader( const void *data ) const
{
	const ngdheader_t *h;

	h = (const ngdheader_t *)data;

    if ( h->validation.ident != IDENT ) {
        Con_Printf( COLOR_RED "LoadArchiveFile: failed to load save, header has incorrect identifier.\n" );
		return false;
    }

    if ( h->validation.version.m_nVersionMajor != _NOMAD_VERSION
    || h->validation.version.m_nVersionUpdate != _NOMAD_VERSION_UPDATE
    || h->validation.version.m_nVersionPatch != _NOMAD_VERSION_PATCH ) {
        Con_Printf( COLOR_RED "LoadArchiveFile: failed to load save, header has incorrect version.\n" );
        return false;
    }

	return true;
}

qboolean CGameArchive::LoadArchiveFile( const char *filename, uint64_t index )
{
	ngd_file_t *file;
	ngdheader_t header;
    uint32_t i, nameLength, j;
    fileHandle_t hFile;
	ngdsection_read_t *section;
	ngdfield_t *field;
	void *data;
	
	hFile = FS_FOpenRead( filename );
	if ( hFile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to open save file '%s'!\n", filename );
		return false;
	}
	
	FS_Read( &header, sizeof(header), hFile );
	if ( !ValidateHeader( &header ) ) {
		return false;
	}
	
	file = (ngd_file_t *)Z_Malloc( sizeof(*file), TAG_SAVEFILE );
	
	N_strncpyz( file->name, filename, sizeof(file->name) );
	memcpy( &file->header, &header, sizeof(header) );
	
	file->m_pSectionList = (ngdsection_read_t *)Z_Malloc( sizeof(*file->m_pSectionList) * header.numSections, TAG_SAVEFILE );
	
	section = file->m_pSectionList;
	for ( i = 0; i < header.numSections; i++ ) {
		FS_Read( &section->nameLength, sizeof(section->nameLength), hFile );
		
		if ( section->nameLength >= MAX_STRING_CHARS ) {
			N_Error( ERR_DROP, "(LoadArchiveFile) save file '%s' has corrupt section\n", filename );
		}
		
		FS_Read( section->name, section->nameLength, hFile );
		FS_Read( &section->size, sizeof(section->size), hFile );
		FS_Read( &section->numFields, sizeof(section->numFields), hFile );
		
		if ( !section->size ) {
			N_Error( ERR_DROP, "(LoadArchiveFile) bad section size at %s", section->name );
		}
		
		section->m_pFieldList = (ngdfield_t *)Z_Malloc( sizeof( ngdfield_t ) * section->numFields, TAG_SAVEFILE );

		field = section->m_pFieldList;
		for ( j = 0; j < section->numFields; j++ ) {
			FS_Read( &field->nameLength, sizeof(field->nameLength), hFile );

			if ( !field->nameLength || field->nameLength >= MAX_STRING_CHARS ) {
				N_Error( ERR_DROP, "(LoadArchiveFile) failed to load save '%s', field name length is corrupt at section '%s' (index %i)",
					filename, section->name, i );
			}
			
			field->name = (char *)Z_Malloc( field->nameLength, TAG_SAVEFILE );
			
			FS_Read( field->name, field->nameLength, hFile );
			FS_Read( &field->type, sizeof( field->type ), hFile );
			
			switch ( field->type ) {
			case FT_ARRAY:
				FS_Read( &field->dataSize, sizeof( field->dataSize ), hFile );
				field->data.str = (char *)Z_Malloc( field->dataSize, TAG_SAVEFILE );
				data = field->data.str;
				break;
			case FT_CHAR:
				field->dataSize = sizeof(int8_t);
				data = &field->data.s8;
				break;
			case FT_SHORT:
				field->dataSize = sizeof(int16_t);
				data = &field->data.s16;
				break;
			case FT_INT:
				field->dataSize = sizeof(int32_t);
				data = &field->data.s32;
				break;
			case FT_LONG:
				field->dataSize = sizeof(int64_t);
				data = &field->data.s64;
				break;
			case FT_UCHAR:
				field->dataSize = sizeof(uint8_t);
				data = &field->data.u8;
				break;
			case FT_USHORT:
				field->dataSize = sizeof(uint16_t);
				data = &field->data.u16;
				break;
			case FT_UINT:
				field->dataSize = sizeof(uint32_t);
				data = &field->data.u32;
				break;
			case FT_ULONG:
				field->dataSize = sizeof(uint64_t);
				data = &field->data.u64;
				break;
			case FT_FLOAT:
				field->dataSize = sizeof(float);
				data = &field->data.f;
				break;
			case FT_VECTOR2:
				field->dataSize = sizeof(vec2_t);
				data = field->data.v2;
				break;
			case FT_VECTOR3:
				field->dataSize = sizeof(vec3_t);
				data = field->data.v3;
				break;
			case FT_VECTOR4:
				field->dataSize = sizeof(vec4_t);
				data = field->data.v4;
				break;
			case FT_STRING:
				FS_Read( &field->dataSize, sizeof(field->dataSize), hFile );
				if ( !field->dataSize ) {
					N_Error( ERR_DROP, "(LoadArchiveFile) failed to load save '%s', field '%s', dataSize is corrupt", filename, field->name );
				}

				field->data.str = (char *)Z_Malloc( field->dataSize, TAG_SAVEFILE );
				data = field->data.str;
				break;
			};

			FS_Read( data, field->dataSize, hFile );
			
			field++;
		}
		
		section++;
	}
	
	m_pArchiveCache[index] = file;
	
	return qtrue;
}

void G_InitArchiveHandler( void )
{
	if ( g_pArchiveHandler ) {
		return;
	}

	g_pArchiveHandler = (CGameArchive *)Hunk_Alloc( sizeof(*g_pArchiveHandler), h_high );
	::new ( g_pArchiveHandler ) CGameArchive();

	g_pArchiveHandler->m_pArchiveFileList = FS_ListFiles( "SaveData", ".ngd", &g_pArchiveHandler->m_nArchiveFiles );
}

void G_ShutdownArchiveHandler( void ) {
	Con_Printf( "G_ShutdownArchiveHandler: clearing save file cache...\n" );

	g_pArchiveHandler = NULL;
}

const char **CGameArchive::GetSaveFiles( uint64_t *nFiles ) const {
	*nFiles = m_nArchiveFiles;
	return (const char **)m_pArchiveFileList;
}

void CGameArchive::BeginSaveSection( const char *name )
{
	uint32_t nameLength = strlen( name );
	
	if ( nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "CGameArchive::AddSection: section name '%s' is longer than %i characters, please shorten, like seriously",
			name, MAX_STRING_CHARS );
	}
	
	Con_DPrintf( "Adding section '%s' to archive file...\n", name );

	m_hFile = FS_FOpenWrite( va( "SaveData/parts/%s.prt", name ) );
	if ( m_hFile == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "CGameArchive::BeginSaveSection: failed to create save section file '%s'", name );
	}

	m_Section.size = 0;
	m_Section.numFields = 0;
	N_strncpyz( m_Section.name, name, sizeof(m_Section.name) );

	nameLength++;

	FS_Write( &nameLength, sizeof(nameLength), m_hFile );
	FS_Write( m_Section.name, nameLength, m_hFile );
	FS_Write( &m_Section.size, sizeof(m_Section.size), m_hFile );
	FS_Write( &m_Section.numFields, sizeof(m_Section.numFields), m_hFile );

	m_nSectionDepth++;
}

void CGameArchive::EndSaveSection( void )
{
	const uint32_t nameLength = strlen( m_Section.name ) + 1;
	
	FS_FileSeek( m_hFile, 0, FS_SEEK_SET );
	FS_Write( &nameLength, sizeof(nameLength), m_hFile );
	FS_Write( m_Section.name, nameLength, m_hFile );
	FS_Write( &m_Section.size, sizeof(m_Section.size), m_hFile );
	FS_Write( &m_Section.numFields, sizeof(m_Section.numFields), m_hFile );
	
	FS_FClose( m_hFile );
	m_hFile = FS_INVALID_HANDLE;
	
	m_nSectionDepth--;
	m_nSections++;
}

void CGameArchive::AddField( const char *name, int32_t type, const void *data, uint32_t dataSize )
{
	ngdfield_t field;
	int64_t i;
	
	m_Section.numFields++;

	field.type = type;
	field.dataSize = dataSize;
	field.nameLength = strlen( name ) + 1;

	if ( field.nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: name '%s' too long", __func__, name );
	}
	
	for ( i = 0; i < m_nSectionDepth; i++ ) {
		Con_DPrintf( "- " );
	}
	Con_DPrintf( "Adding field %s to save file.\n", name );

	FS_Write( &field.nameLength, sizeof(field.nameLength), m_hFile );
	FS_Write( name, field.nameLength, m_hFile );
	FS_Write( &field.type, sizeof(field.type), m_hFile );
	FS_Write( data, dataSize, m_hFile );
}

void CGameArchive::SaveFloat( const char *name, float data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}

	AddField( name, FT_FLOAT, &data, sizeof(data) );
}

void CGameArchive::SaveByte( const char *name, uint8_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_UCHAR, &data, sizeof(data) );
}
void CGameArchive::SaveUShort( const char *name, uint16_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_USHORT, &data, sizeof(data) );
}
void CGameArchive::SaveUInt( const char *name, uint32_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_UINT, &data, sizeof(data) );
}
void CGameArchive::SaveULong( const char *name, uint64_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_ULONG, &data, sizeof(data) );
}

void CGameArchive::SaveChar( const char *name, int8_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_CHAR, &data, sizeof(data) );
}
void CGameArchive::SaveShort( const char *name, int16_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_SHORT, &data, sizeof(data) );
}
void CGameArchive::SaveInt( const char *name, int32_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_INT, &data, sizeof(data) );
}
void CGameArchive::SaveLong( const char *name, int64_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_LONG, &data, sizeof(data) );
}

void CGameArchive::SaveVec2( const char *name, const vec2_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_VECTOR2, data, sizeof(vec2_t) );
}

void CGameArchive::SaveVec3( const char *name, const vec3_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_VECTOR3, data, sizeof(vec3_t) );
}

void CGameArchive::SaveVec4( const char *name, const vec4_t data ) {
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	
	AddField( name, FT_VECTOR4, data, sizeof(vec4_t) );
}

void CGameArchive::SaveCString( const char *name, const char *data ) {
	ngdfield_t field;

	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !data ) {
		N_Error( ERR_DROP, "%s: data is NULL", __func__ );
	}

	field.nameLength = strlen( name ) + 1;
	field.type = FT_STRING;
	field.dataSize = strlen( data ) + 1;
	if ( field.nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: name '%s' too long", __func__, name );
	}
	if ( field.dataSize >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: string too long", __func__ );
	}

	FS_Write( &field.nameLength, sizeof(field.nameLength), m_hFile );
	FS_Write( name, field.nameLength, m_hFile );
	FS_Write( &field.type, sizeof(field.type), m_hFile );
	FS_Write( &field.dataSize, sizeof(field.dataSize), m_hFile );
	FS_Write( data, field.dataSize, m_hFile );
}

void CGameArchive::SaveString( const char *name, const string_t *pData ) {
	ngdfield_t field;

	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !pData ) {
		N_Error( ERR_DROP, "%s: data is NULL", __func__ );
	}

	field.nameLength = strlen( name ) + 1;
	field.type = FT_STRING;
	field.dataSize = pData->size() + 1;
	if ( field.nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: name '%s' too long", __func__, name );
	}

	FS_Write( &field.nameLength, sizeof(field.nameLength), m_hFile );
	FS_Write( name, field.nameLength, m_hFile );
	FS_Write( &field.type, sizeof(field.type), m_hFile );
	FS_Write( &field.dataSize, sizeof(field.dataSize), m_hFile );
	FS_Write( pData->data(), field.dataSize, m_hFile );
}

void CGameArchive::SaveArray( const char *name, const CScriptArray *pData ) {
	ngdfield_t field;

	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !pData ) {
		N_Error( ERR_DROP, "%s: data is NULL", __func__ );
	}

	field.nameLength = strlen( name ) + 1;
	field.type = FT_STRING;
	field.dataSize = pData->GetSize() * g_pModuleLib->GetScriptEngine()->GetTypeInfoById( pData->GetElementTypeId() )->GetSize();
	if ( field.nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: name '%s' too long", __func__, name );
	}

	FS_Write( &field.nameLength, sizeof( field.nameLength ), m_hFile );
	FS_Write( name, field.nameLength, m_hFile );
	FS_Write( &field.type, sizeof( field.type ), m_hFile );
	FS_Write( &field.dataSize, sizeof( field.dataSize ), m_hFile );
	FS_Write( pData->GetBuffer(), field.dataSize, m_hFile );
}

const ngdfield_t *CGameArchive::FindField( const char *name, int32_t type, nhandle_t hSection ) const
{
	int64_t i;
	const ngdfield_t *f;

	f = NULL;
	for ( i = 0; i < m_pCurrentArchive->m_pSectionList[ hSection ].numFields; i++ ) {
		f = &m_pCurrentArchive->m_pSectionList[ hSection ].m_pFieldList[i];
		if ( !N_stricmp( f->name, name ) ) {
			break;
		}
		f = NULL;
	}

	if ( !f ) {
		N_Error( ERR_DROP, "CGameArchive::FindField: incompatible mod with save file, couldn't find field '%s'", name );
	}
	
	if ( f->type != type ) {
		N_Error( ERR_DROP, "CGameArchive::FindField: save file corrupt or incompatible mod, field type doesn't match type given for '%s'", name );
	}

	return f;
}

float CGameArchive::LoadFloat( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_FLOAT, hSection );
	
	return field->data.f;
}

uint8_t CGameArchive::LoadByte( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_UCHAR, hSection );
	
	return field->data.u8;
}

uint16_t CGameArchive::LoadUShort( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_USHORT, hSection );
	
	return field->data.u16;
}

uint32_t CGameArchive::LoadUInt( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_UINT, hSection );
	
	return field->data.u32;
}

uint64_t CGameArchive::LoadULong( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ULONG, hSection );
	
	return field->data.u64;
}

int8_t CGameArchive::LoadChar( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_CHAR, hSection );
	
	return field->data.s8;
}

int16_t CGameArchive::LoadShort( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_SHORT, hSection );
	
	return field->data.s16;
}

int32_t CGameArchive::LoadInt( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_INT, hSection );

	return field->data.s32;
}

int64_t CGameArchive::LoadLong( const char *name, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_LONG, hSection );
	
	return field->data.s64;
}

void CGameArchive::LoadVec2( const char *name, vec2_t data, nhandle_t hSection )
{
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_VECTOR2, hSection );
	
	VectorCopy2( data, field->data.v2 );
}

void CGameArchive::LoadVec3( const char *name, vec3_t data, nhandle_t hSection )
{
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_VECTOR3, hSection );
	
	VectorCopy( data, field->data.v3 );
}

void CGameArchive::LoadVec4( const char *name, vec4_t data, nhandle_t hSection )
{
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}

	field = FindField( name, FT_VECTOR4, hSection );
	
	VectorCopy4( data, field->data.v2 );
}

void CGameArchive::LoadCString( const char *name, char *pBuffer, int32_t maxLength, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_STRING, hSection );
	
	N_strncpyz( pBuffer, field->data.str, maxLength );
}

void CGameArchive::LoadString( const char *name, string_t *pString, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_STRING, hSection );
	
	if ( pString->size() < field->dataSize ) {
		pString->resize( field->dataSize );
	}
	N_strncpyz( pString->data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadArray( const char *name, CScriptArray *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	const asITypeInfo *info;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );
	info = g_pModuleLib->GetScriptEngine()->GetTypeInfoById( pData->GetElementTypeId() );

	// this should never happen
	if ( ( pData->GetSize() * info->GetSize() ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for script array", __func__ );
	}
	if ( pData->GetSize() * info->GetSize() < field->dataSize ) {
		pData->Resize( field->dataSize / info->GetSize() );
	}
	memcpy( pData->GetBuffer(), field->data.str, field->dataSize );
}

bool CGameArchive::Save( void )
{
	const char *path;
	ngdheader_t header;
	char **partFiles;
	uint64_t nPartFiles;
	union {
		void *v;
		char *b;
	} f;
	uint64_t length, i;

	if ( m_nSectionDepth ) {
		N_Error( ERR_DROP, "CGameArchive::Save: called when writing a section" );
	}
	
	path = FS_BuildOSPath( NULL, "SaveData", Cvar_VariableString( "sg_savename" ) );

	m_hFile = FS_FOpenWrite( path );
	if ( m_hFile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to create save file '%s'!\n", path );
		return false;
	}
	
	g_pModuleLib->ModuleCall( sgvm, ModuleOnSaveGame, 0 );

	partFiles = FS_ListFiles( va( "SaveData/%s/parts", Cvar_VariableString( "fs_basegame" ) ), ".prt", &nPartFiles );
	
	memset( &header, 0, sizeof(header) );
	N_strncpyz( header.gamedata.mapName, Cvar_VariableString( "g_mapname" ), sizeof(header.gamedata.mapName) );
	header.gamedata.diff = (gamedif_t)Cvar_VariableInteger( "sg_difficulty" );

	N_strncpyz( header.gamedata.bffName, Cvar_VariableString( "g_modname" ), sizeof(header.gamedata.bffName) );
	header.gamedata.levelIndex = Cvar_VariableInteger( "g_levelIndex" );
	
	header.validation.ident = IDENT;
	header.validation.version.m_nVersionMajor = _NOMAD_VERSION;
	header.validation.version.m_nVersionUpdate = _NOMAD_VERSION_UPDATE;
	header.validation.version.m_nVersionPatch = _NOMAD_VERSION_PATCH;
	FS_Write( &header, sizeof(header), m_hFile );

	for ( i = 0; i < nPartFiles; i++ ) {
		length = FS_LoadFile( partFiles[i], &f.v );
		if ( !length || !f.v ) {
			N_Error( ERR_DROP, "CGameArchive::Save: failed to load save section file '%s'", partFiles[i] );
		}

		// the section data is already there, so we just need to write it to the actual save file
		FS_Write( f.v, length, m_hFile );

		FS_FreeFile( f.v );
	}
	
	FS_FClose( m_hFile );
	
	//
	// save to steam
	//

	m_nSections = 0;

	return true;
}

nhandle_t CGameArchive::GetSection( const char *name )
{
	ngdsection_read_t *section;
	ngd_file_t *file;
	int64_t i;

	section = NULL;
	for ( i = 0; i < file->header.numSections; i++ ) {
		if ( !N_stricmp( file->m_pSectionList[i].name, name ) ) {
			section = &file->m_pSectionList[i];
			break;
		}
	}

	if ( !section ) {
		N_Error( ERR_DROP, "CGameArchive::GetSection: compatibility issue with save file section '%s', section not found in file", name );
	}
	
	return i;
}

bool CGameArchive::LoadPartial( const char *filename, gamedata_t *gd )
{
    fileHandle_t f;
    ngdheader_t header;

    if ( FS_FileIsInBFF( filename ) ) {
        N_Error(ERR_FATAL, "Savefile '%s' was in a bff, bffs are for game resources, not save data", filename);
    }

    f = FS_FOpenRead( filename );
    if (f == FS_INVALID_HANDLE) {
        return false;
    }

    //
    // validate the header
    //

    if ( FS_FileLength( f ) < sizeof(header) ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile because the file is too small to contain a header.\n" );
        return false;
    }

    FS_Read( &header, sizeof(header), f );

    if ( !ValidateHeader( &header ) ) {
        return false;
    }

    memcpy( gd, &header.gamedata, sizeof(*gd) );

    FS_FClose( f );

    return true;
}

bool CGameArchive::Load( const char *name )
{
	ngd_file_t *file;
	uint64_t i;
	
	file = NULL;
	m_pCurrentArchive = NULL;
	
	for ( i = 0; i < m_nArchiveFiles; i++ ) {
		if ( !N_stricmp( name, m_pArchiveCache[i]->name ) ) {
			m_pCurrentArchive = m_pArchiveCache[i];
			break;
		}
	}
	
	if ( !file ) {
		N_Error( ERR_DROP, "CGameArchive::Load: attempted to load non-existing save file (... HOW?)" );
	}

	N_strncpyz( m_pCurrentArchive->name, name, sizeof(m_pCurrentArchive->name) );
	if ( !LoadArchiveFile( m_pCurrentArchive->name, i ) ) {
		N_Error( ERR_DROP, "CGameArchive::Load: failed to load an invalid save file" );
	}

	g_pModuleLib->ModuleCall( sgvm, ModuleOnLoadGame, 0 );

	return true;
}
