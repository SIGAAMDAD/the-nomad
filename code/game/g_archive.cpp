// g_archive.cpp -- no game data archiving allowed within the vm to allow various mods and vanilla saves to work with each other

#include "g_game.h"
#include "g_archive.h"
#include "../ui/ui_lib.h"

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
	ngdheader_t header;
    uint32_t i, j;
	int64_t nameLength, size, numFields;
	uint64_t bufSize;
    fileHandle_t hFile;
	ngdsection_read_t *section;
	ngdfield_t *field;
	ngd_file_t *file;
	char name[MAX_STRING_CHARS];
	
	hFile = FS_FOpenRead( filename );
	if ( hFile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to open save file '%s'!\n", filename );
		return false;
	}
	
	FS_Read( &header.validation, sizeof( header.validation ), hFile );
	if ( !ValidateHeader( &header ) ) {
		return false;
	}

	FS_Read( &header.numSections, sizeof( header.numSections ), hFile );
	FS_Read( header.gamedata.mapname, sizeof( header.gamedata.mapname ), hFile );
	FS_Read( &header.gamedata.dif, sizeof( header.gamedata.dif ), hFile );
	FS_Read( &header.gamedata.numMods, sizeof( header.gamedata.numMods ), hFile );

	FS_FileSeek( hFile, sizeof( *header.gamedata.modList ) * MAX_NPATH * header.gamedata.numMods, FS_SEEK_CUR );

	file = (ngd_file_t *)Hunk_Alloc( sizeof( *file ) * header.numSections, h_low );
	file->m_pSectionList = (ngdsection_read_t *)( file + 1 );

	file->m_nSections = header.numSections;
	section = file->m_pSectionList;
	
	for ( i = 0; i < header.numSections; i++ ) {
		FS_Read( &nameLength, sizeof( nameLength ), hFile );
		
		if ( nameLength >= MAX_STRING_CHARS ) {
			N_Error( ERR_DROP, "(LoadArchiveFile) save file '%s' has corrupt section\n", filename );
		}
		
		FS_Read( name, nameLength, hFile );
		FS_Read( &size, sizeof( size ), hFile );
		FS_Read( &numFields, sizeof( numFields ), hFile );
		
		if ( !size ) {
			N_Error( ERR_DROP, "(LoadArchiveFile) bad section size at %s", section->name );
		}

		section->m_FieldCache.reserve( numFields );
		section->numFields = numFields;
		N_strncpyz( section->name, name, sizeof( section->name ) );
		section->nameLength = nameLength;

		for ( j = 0; j < section->numFields; j++ ) {
			FS_Read( &nameLength, sizeof( field->nameLength ), hFile );

			if ( !nameLength || nameLength >= MAX_STRING_CHARS ) {
				N_Error( ERR_DROP, "(LoadArchiveFile) failed to load save '%s', field name length is corrupt at section '%s' (index %i)",
					filename, section->name, i );
			}
			
			field = (ngdfield_t *)Hunk_Alloc( PAD( sizeof( *field ) + nameLength, sizeof( uintptr_t ) ), h_low );
			field->name = (char *)( field + 1 );
			field->dataOffset = FS_FileTell( hFile );
			field->nameLength = nameLength;

			FS_Read( field->name, field->nameLength, hFile );
			FS_Read( &field->type, sizeof( field->type ), hFile );
			
			switch ( field->type ) {
			case FT_ARRAY:
				FS_Read( &field->dataSize, sizeof( field->dataSize ), hFile );
				break;
			case FT_CHAR:
				field->dataSize = sizeof( int8_t );
				break;
			case FT_SHORT:
				field->dataSize = sizeof( int16_t );
				break;
			case FT_INT:
				field->dataSize = sizeof( int32_t );
				break;
			case FT_LONG:
				field->dataSize = sizeof( int64_t );
				break;
			case FT_UCHAR:
				field->dataSize = sizeof( uint8_t );
				break;
			case FT_USHORT:
				field->dataSize = sizeof( uint16_t );
				break;
			case FT_UINT:
				field->dataSize = sizeof( uint32_t );
				break;
			case FT_ULONG:
				field->dataSize = sizeof( uint64_t );
				break;
			case FT_FLOAT:
				field->dataSize = sizeof( float );
				break;
			case FT_VECTOR2:
				field->dataSize = sizeof( vec2_t );
				break;
			case FT_VECTOR3:
				field->dataSize = sizeof( vec3_t );
				break;
			case FT_VECTOR4:
				field->dataSize = sizeof( vec4_t );
				break;
			case FT_STRING:
				FS_Read( &field->dataSize, sizeof( field->dataSize ), hFile );
				if ( !field->dataSize ) {
					N_Error( ERR_DROP, "(LoadArchiveFile) failed to load save '%s', field '%s', dataSize is corrupt", filename, field->name );
				}
				break;
			};

			section->m_FieldCache.try_emplace( field->name, field );

			FS_FileSeek( hFile, field->dataSize, FS_SEEK_CUR );
		}

		section++;
	}

	FS_FileSeek( hFile, 0, FS_SEEK_SET );

	m_pArchiveCache[ index ] = file;
	m_hFile = hFile;
	
	return qtrue;
}

static void G_SaveGame_f( void )
{
	const char *filename;
	char path[MAX_NPATH];

	filename = Cmd_Argv( 1 );

	if ( *filename ) {
		COM_StripExtension( filename, path, sizeof( path ) );
		COM_DefaultExtension( path, sizeof( path ), ".ngd" );
		filename = Cvar_VariableString( "sgame_SaveName" );
	} else {
		Com_snprintf( path, sizeof( path ) - 1, "%s", Cvar_VariableString( "sgame_SaveName" ) );
		COM_DefaultExtension( path, sizeof( path ), ".ngd" );
		filename = path;
	}
	g_pArchiveHandler->Save( path );
}

CGameArchive::CGameArchive( void )
{
	uint64_t i, size;
	char *namePtr;
	char **fileList;

	Con_Printf( "G_InitArchiveHandler: initializing save file cache...\n" );

	fileList = FS_ListFiles( "SaveData", ".ngd", &m_nArchiveFiles );
	m_pArchiveCache = (ngd_file_t **)Hunk_Alloc( sizeof( *m_pArchiveCache ) * m_nArchiveFiles, h_low );

	m_pArchiveFileList = (char **)Hunk_Alloc( sizeof( *fileList ) * m_nArchiveFiles, h_low );
	for ( i = 0; i < m_nArchiveFiles; i++ ) {
		size = strlen( fileList[i] ) + 1;
		m_pArchiveFileList[i] = (char *)Hunk_Alloc( size, h_low );
		N_strncpyz( m_pArchiveFileList[i], fileList[i], size );
		LoadArchiveFile( fileList[i], i );

		Con_Printf( "...Cached save file '%s'\n", fileList[i] );
	}

	FS_FreeFileList( fileList );

	Cmd_AddCommand( "sgame.save_game", G_SaveGame_f );
}

void G_InitArchiveHandler( void )
{
	if ( g_pArchiveHandler ) {
		return;
	}

	g_pArchiveHandler = new ( Hunk_Alloc( sizeof( *g_pArchiveHandler ), h_low ) ) CGameArchive();
}

void G_ShutdownArchiveHandler( void ) {
	g_pArchiveHandler = NULL;
}

const char **CGameArchive::GetSaveFiles( uint64_t *nFiles ) const {
	*nFiles = m_nArchiveFiles;
	return (const char **)m_pArchiveFileList;
}

void CGameArchive::BeginSaveSection( const char *moduleName, const char *name )
{
	const char *path;
	uint32_t nameLength = strlen( name );
	
	if ( nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "CGameArchive::AddSection: section name '%s' is longer than %i characters, please shorten, like seriously",
			name, MAX_STRING_CHARS );
	}
	
	Con_DPrintf( "Adding section '%s' to archive file...\n", name );

	path = va( "SaveData/%s/%s.prt", moduleName, name );
	m_hFile = FS_FOpenWrite( path );
	if ( m_hFile == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "CGameArchive::BeginSaveSection: failed to create save section file '%s'", path );
	}

	m_Section.size = 0;
	m_Section.numFields = 0;
	N_strncpyz( m_Section.name, name, sizeof( m_Section.name ) );

	nameLength++;

	FS_Write( &nameLength, sizeof( nameLength ), m_hFile );
	FS_Write( m_Section.name, nameLength, m_hFile );
	FS_Write( &m_Section.size, sizeof( m_Section.size ), m_hFile );
	FS_Write( &m_Section.numFields, sizeof( m_Section.numFields ), m_hFile );

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

void CGameArchive::SaveArray( const char *func, const char *name, const void *pData, uint32_t nBytes ) {
	ngdfield_t field;

	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", func );
	}
	if ( !pData ) {
		N_Error( ERR_DROP, "%s: data is NULL", func );
	}

	field.nameLength = strlen( name ) + 1;
	field.type = FT_STRING;
	field.dataSize = nBytes;
	if ( field.nameLength >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "%s: name '%s' too long", func, name );
	}

	FS_Write( &field.nameLength, sizeof( field.nameLength ), m_hFile );
	FS_Write( name, field.nameLength, m_hFile );
	FS_Write( &field.type, sizeof( field.type ), m_hFile );
	FS_Write( &field.dataSize, sizeof( field.dataSize ), m_hFile );
	FS_Write( pData, nBytes, m_hFile );
}

void CGameArchive::SaveInt8Array( const char *name, const aatc::container::tempspec::vector<int8_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( int8_t ) );
}

void CGameArchive::SaveInt16Array( const char *name, const aatc::container::tempspec::vector<int16_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( int16_t ) );
}

void CGameArchive::SaveInt32Array( const char *name, const aatc::container::tempspec::vector<int32_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( int32_t ) );
}

void CGameArchive::SaveInt64Array( const char *name, const aatc::container::tempspec::vector<int64_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( int64_t ) );
}

void CGameArchive::SaveUInt8Array( const char *name, const aatc::container::tempspec::vector<uint8_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( uint8_t ) );
}

void CGameArchive::SaveUInt16Array( const char *name, const aatc::container::tempspec::vector<uint16_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( uint16_t ) );
}

void CGameArchive::SaveUInt32Array( const char *name, const aatc::container::tempspec::vector<uint32_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( uint32_t ) );
}

void CGameArchive::SaveUInt64Array( const char *name, const aatc::container::tempspec::vector<uint64_t> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( uint64_t ) );
}

void CGameArchive::SaveFloatArray( const char *name, const aatc::container::tempspec::vector<float> *pData ) {
	SaveArray( __func__, name, pData->container.data(), pData->container.size() * sizeof( float ) );
}

void CGameArchive::SaveArray( const char *name, const CScriptArray *pData ) {
	SaveArray( __func__, name, pData->GetBuffer(), pData->GetSize()
		* g_pModuleLib->GetScriptEngine()->GetTypeInfoById( pData->GetElementTypeId() )->GetSize() );
}

const ngdfield_t *CGameArchive::FindField( const char *name, int32_t type, nhandle_t hSection ) const
{
	const ngdsection_read_t *section;

	section = &m_pArchiveCache[ m_nCurrentArchive ]->m_pSectionList[ hSection ];
	const auto it = section->m_FieldCache.find( name );

	if ( it == section->m_FieldCache.end() ) {
		N_Error( ERR_DROP, "CGameArchive::FindField: incompatible mod with save file, couldn't find field '%s'", name );
	}
	if ( it->second->type != type ) {
		N_Error( ERR_DROP, "CGameArchive::FindField: save file corrupt or incompatible mod, field type doesn't match type given for '%s'", name );
	}

	FS_FileSeek( m_hFile, it->second->dataOffset, FS_SEEK_SET );

	return it->second;
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}

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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
	
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
	if ( !FS_Read( (void *)&field->data, field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}

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

	// FIXME:
	
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
	memset( pString->data(), 0, pString->size() );
	if ( !FS_Read( pString->data(), field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, name );
	}
}

void CGameArchive::LoadInt8Array( const char *name, aatc::container::tempspec::vector<int8_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( int8_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( int8_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( int8_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadInt16Array( const char *name, aatc::container::tempspec::vector<int16_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( int16_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( int16_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( int16_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadInt32Array( const char *name, aatc::container::tempspec::vector<int32_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( int32_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( int32_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( int32_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadInt64Array( const char *name, aatc::container::tempspec::vector<int64_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( int64_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( int64_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( int64_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadUInt8Array( const char *name, aatc::container::tempspec::vector<uint8_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( uint8_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( uint8_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( uint8_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadUInt16Array( const char *name, aatc::container::tempspec::vector<uint16_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( uint16_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( uint16_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( uint16_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadUInt32Array( const char *name, aatc::container::tempspec::vector<uint32_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( uint32_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( uint32_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( uint32_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadUInt64Array( const char *name, aatc::container::tempspec::vector<uint64_t> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( uint64_t ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( uint64_t ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( uint64_t ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadFloatArray( const char *name, aatc::container::tempspec::vector<float> *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	
	if ( !name ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}
	
	field = FindField( name, FT_ARRAY, hSection );

	// this should never happen
	if ( ( pData->container.size() * sizeof( float ) ) % field->dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module vector (funny field size)", __func__ );
	}
	if ( pData->container.size() * sizeof( float ) < field->dataSize ) {
		pData->container.resize( field->dataSize / sizeof( float ) );
	}
	memcpy( pData->container.data(), field->data.str, field->dataSize );
}

void CGameArchive::LoadArray( const char *pszName, CScriptArray *pData, nhandle_t hSection ) {
	const ngdfield_t *field;
	uint32_t dataSize;

	if ( !pszName ) {
		N_Error( ERR_DROP, "%s: name is NULL", __func__ );
	}
	if ( !hSection ) {
		N_Error( ERR_DROP, "%s: hSection is invalid", __func__ );
	}

	field = FindField( pszName, FT_ARRAY, hSection );

	dataSize = g_pModuleLib->GetScriptEngine()->GetTypeInfoById( pData->GetElementTypeId() )->GetSize();

	// this should never happen
	if ( field->dataSize % dataSize ) {
		N_Error( ERR_DROP, "%s: bad data type for module array (funny field size)", __func__ );
	}
	if ( pData->GetSize() < ( field->dataSize / dataSize ) ) {
		pData->Resize( field->dataSize / dataSize );
	}

	Con_DPrintf( "Successfully loaded array field '%s' containing %u bytes.\n", field->name, field->dataSize );

	if ( !FS_Read( pData->GetBuffer(), field->dataSize, m_hFile ) ) {
		N_Error( ERR_DROP, "%s: failed to read field '%s'", __func__, pszName );
	}
}

// NOTE: NEVER CHANGE THIS, IF U DO, YOU'LL BREAK SAVEFILES!!!!
#define HEADER_SIZE ( sizeof( ngdvalidation_t ) + ( sizeof( uint64_t ) * 2 ) + MAX_NPATH ) + sizeof( gamedif_t )

bool CGameArchive::Save( const char *filename )
{
	const char *path;
	ngdheader_t header;
	char **partFiles;
	uint64_t nPartFiles;
	union {
		void *v;
		char *b;
	} f;
	uint64_t length, i, size;
	char *namePtr;

	PROFILE_FUNCTION();

	if ( m_nSectionDepth ) {
		N_Error( ERR_DROP, "CGameArchive::Save: called when writing a section" );
	}
	
	g_pModuleLib->ModuleCall( sgvm, ModuleOnSaveGame, 0 );

	path = va( "SaveData/%s", filename );
	m_hFile = FS_FOpenWrite( path );
	if ( m_hFile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to create save file '%s'!\n", path );
		return false;
	}

	const UtlVector<CModuleInfo *>& loadList = g_pModuleLib->GetLoadList();
	
	memset( &header, 0, sizeof( header ) );
	N_strncpyz( header.gamedata.mapname, Cvar_VariableString( "mapname" ), sizeof( header.gamedata.mapname ) );
	header.gamedata.dif = (gamedif_t)Cvar_VariableInteger( "sgame_Difficulty" );
	header.gamedata.numMods = loadList.size();

	for ( const auto& it : loadList ) {
		if ( !it->m_pHandle->IsValid() ) {
			header.gamedata.numMods--;
		}
	}
	
	header.validation.ident = IDENT;
	header.validation.version.m_nVersionMajor = _NOMAD_VERSION;
	header.validation.version.m_nVersionUpdate = _NOMAD_VERSION_UPDATE;
	header.validation.version.m_nVersionPatch = _NOMAD_VERSION_PATCH;

	FS_Write( &header.validation, sizeof( header.validation ), m_hFile );
	FS_Write( &header.numSections, sizeof( header.numSections ), m_hFile );
	FS_Write( header.gamedata.mapname, sizeof( header.gamedata.mapname ), m_hFile );
	FS_Write( &header.gamedata.dif, sizeof( header.gamedata.dif ), m_hFile );
	FS_Write( &header.gamedata.numMods, sizeof( header.gamedata.numMods ), m_hFile );

	for ( i = 0; i < header.gamedata.numMods; i++ ) {
		if ( loadList[i]->m_pHandle->IsValid() ) {
			FS_Write( loadList[i]->m_szName, sizeof( loadList[i]->m_szName ), m_hFile );
			FS_Write( &loadList[i]->m_nModVersionMajor, sizeof( loadList[i]->m_nModVersionMajor ), m_hFile );
			FS_Write( &loadList[i]->m_nModVersionUpdate, sizeof( loadList[i]->m_nModVersionUpdate ), m_hFile );
			FS_Write( &loadList[i]->m_nModVersionPatch, sizeof( loadList[i]->m_nModVersionPatch ), m_hFile );
		}
	}

	for ( const auto& it : loadList ) {
		partFiles = FS_ListFiles( FS_BuildOSPath( FS_GetHomePath(), NULL, va( "SaveData/%s/", it->m_szName ) ), ".prt", &nPartFiles );

		for ( i = 0; i < nPartFiles; i++ ) {
			length = FS_LoadFile( partFiles[i], &f.v );
			if ( !length || !f.v ) {
				N_Error( ERR_DROP, "CGameArchive::Save: failed to load save section file '%s'", partFiles[i] );
			}

			Con_DPrintf( "Writing save section part '%s' to archive file...\n", partFiles[i] );

			// the section data is already there, so we just need to write it to the actual save file
			FS_Write( f.v, length, m_hFile );
	
			FS_FreeFile( f.v );
		}

		FS_FreeFileList( partFiles );
	}
	
	FS_FClose( m_hFile );
	
	//
	// save to steam
	//

	m_nSections = 0;

	//
	// reload the file list
	//
	Z_FreeTags( TAG_SAVEFILE, TAG_SAVEFILE );
	m_pArchiveCache = NULL;
	m_pArchiveFileList = NULL;
	m_nArchiveFiles = 0;
	m_nCurrentArchive = 0;
	CGameArchive();
	SinglePlayerMenu_Cache();

	return true;
}

nhandle_t CGameArchive::GetSection( const char *name )
{
	ngdsection_read_t *section;
	int64_t i;

	section = NULL;
	for ( i = 0; i < m_pArchiveCache[ m_nCurrentArchive ]->m_nSections; i++ ) {
		if ( !N_strcmp( m_pArchiveCache[ m_nCurrentArchive ]->m_pSectionList[i].name, name ) ) {
			section = &m_pArchiveCache[ m_nCurrentArchive ]->m_pSectionList[i];
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
	uint64_t i, size;
	char *namePtr;

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

    if ( FS_FileLength( f ) < HEADER_SIZE ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile because the file is too small to contain a header.\n" );
        return false;
    }

	FS_Read( &header.validation, sizeof( header.validation ), f );
	FS_Read( &header.numSections, sizeof( header.numSections ), f );
	FS_Read( gd->mapname, sizeof( gd->mapname ), f );
	FS_Read( &gd->dif, sizeof( gd->dif ), f );
	FS_Read( &gd->numMods, sizeof( gd->numMods ), f );

	if ( gd->numMods ) {
		gd->modList = (modlist_t *)Hunk_Alloc( sizeof( *gd->modList ) * gd->numMods, h_high );
		for ( i = 0; i < gd->numMods; i++ ) {
			FS_Read( gd->modList[i].name, sizeof( gd->modList[i].name ), f );
			FS_Read( &gd->modList[i].nVersionMajor, sizeof( gd->modList[i].nVersionMajor ), f );
			FS_Read( &gd->modList[i].nVersionUpdate, sizeof( gd->modList[i].nVersionUpdate ), f );
			FS_Read( &gd->modList[i].nVersionPatch, sizeof( gd->modList[i].nVersionPatch ), f );
		}
	}
	Con_DPrintf(
				"Loaded partial header:\n"
				" [mapname] %s\n"
				" [difficulty] %i\n"
				" [modCount] %lu\n"
				" [modList] "
	, gd->mapname, (int32_t)gd->dif, gd->numMods );

	for ( i = 0; i < gd->numMods; i++ ) {
		Con_DPrintf( "%s", gd->modList[i].name );
		if ( i != gd->numMods - 1 ) {
			Con_DPrintf( ", " );
		}
	}
	Con_DPrintf( "\n" );

    if ( !ValidateHeader( &header ) ) {
        return false;
    }

    FS_FClose( f );

    return true;
}

bool CGameArchive::Load( const char *name )
{
	uint64_t i;

	m_nCurrentArchive = m_nArchiveFiles;
	for ( i = 0; i < m_nArchiveFiles; i++ ) {
		if ( !N_strcmp( name, m_pArchiveCache[i]->name ) ) {
			m_nCurrentArchive = i;
			break;
		}
	}
	
	if ( i == m_nArchiveFiles ) {
		N_Error( ERR_DROP, "CGameArchive::Load: attempted to load non-existing save file (... HOW?)" );
	}

	g_pModuleLib->ModuleCall( sgvm, ModuleOnLoadGame, 0 );

	return true;
}
