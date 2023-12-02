#include "rgl_local.h"
#include <ctype.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static byte s_intensitytable[256];
static uint8_t s_gammatable[256];

int32_t	gl_filter_min = GL_LINEAR;
int32_t	gl_filter_max = GL_NEAREST;

#define FILE_HASH_SIZE 1024
static CTexture *hashTable[MAX_RENDER_TEXTURES];

/*
** R_GammaCorrect
*/
GDR_EXPORT void R_GammaCorrect( byte *buffer, uint64_t bufSize ) {
	uint64_t i;

	for ( i = 0; i < bufSize; i++ ) {
		buffer[i] = s_gammatable[buffer[i]];
	}
}

typedef struct {
    const char *filter;
    int min, mag;
} textureFilterMode_t;

static const textureFilterMode_t filters[] = {
    {"Linear", GL_LINEAR, GL_LINEAR},
    {"Nearest", GL_NEAREST, GL_NEAREST},
    {"Bilinear", GL_NEAREST, GL_LINEAR},
    {"Trilinear", GL_LINEAR, GL_NEAREST}
};

/*
================
return a hash f for the filename
================
*/
GDR_EXPORT uint64_t generateHashValue( const char *fname )
{
	uint32_t i;
	uint64_t hash;
	char letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=(uint64_t)(letter)*(i+119);
		i++;
	}
	hash &= (MAX_RENDER_TEXTURES-1);
	return hash;
}

GDR_EXPORT void R_UpdateTextures(void)
{
    uint32_t i;
    CTexture *t;

    for (i = 0; i < arraylen(filters); i++) {
        if (!N_stricmp(filters[i].filter, r_textureFiltering->s)) {
            break;
        }
    }

    if (i == arraylen(filters)) {
        ri.Cvar_Reset("r_textureFiltering");
        return;
    }

	gl_filter_min = filters[i].min;
	gl_filter_max = filters[i].mag;

    // change all the texture filters
    for (i = 0; i < rg.numTextures; i++) {
        t = rg.textures[i];

	    GL_BindTexture( GL_TEXTURE0, t );

	    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filters[i].min );
	    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filters[i].mag );
        if (glContext.ARB_texture_filter_anisotropic) {
            nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, r_arb_texture_filter_anisotropic->f );
        }

	    GL_BindTexture( GL_TEXTURE0, NULL );
    }
}

/*
===============
R_SumOfUsedImages
===============
*/
GDR_EXPORT uint64_t R_SumOfUsedImages( void )
{
	uint64_t total;
	uint64_t i;

	total = 0;
	for ( i = 0; i < rg.numTextures; i++ ) {
//		if ( rg.textures[i]->frameUsed == rg.frameCount) {
			total += rg.textures[i]->GetWidth() * rg.textures[i]->GetHeight();
//		}
	}

	return total;
}


GDR_EXPORT CTexture* CTexture::CreateImage( const char *name, byte *pic, uint32_t width, uint32_t height, imgType_t type,
                                            imgFlags_t flags, int32_t internalFormat, GLenum picFormat )
{
    CTexture *image;
	uint64_t namelen, hash;
	GLenum glWrapClampMode;
	GLenum dataType;

	namelen = strlen(name);
	if (namelen >= MAX_GDR_PATH) {
		ri.Error(ERR_DROP, "R_CreateImage: name \"%s\" too long", name);
	}
	/*
	if (!strncmp(name, "*lightmap", 9)) {
		isLightmap = qtrue;
	}
	*/

	if (rg.numTextures == MAX_RENDER_TEXTURES) {
		ri.Error(ERR_DROP, "R_CreateImage: MAX_RENDER_TEXTURES hit");
	}

	image = rg.textures[rg.numTextures] = (CTexture *)ri.Hunk_Alloc( sizeof(*image) + namelen, h_low );
    ::new ((void *)image) CTexture();

	nglGenTextures( 1, &image->m_Id );
	rg.numTextures++;

	image->m_pImageName = (char *)(image + 1);
	strcpy( image->m_pImageName, name );

    image->m_Flags = flags;
    image->m_Type = type;
    image->m_InternalFormat = internalFormat;

    if (flags & IMGFLAG_CLAMPTOBORDER) {
        glWrapClampMode = GL_CLAMP_TO_BORDER;
    } else if (flags & IMGFLAG_CLAMPTOEDGE) {
        glWrapClampMode = GL_CLAMP_TO_EDGE;
    } else {
        glWrapClampMode = GL_REPEAT;
    }

    nglBindTexture( GL_TEXTURE_2D, image->m_Id );
    GL_SetObjectDebugName(GL_TEXTURE, image->m_Id, image->m_pImageName, "");

    if (glContext.ARB_texture_filter_anisotropic) {
        nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, r_arb_texture_filter_anisotropic->f );
    }

    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapClampMode );
    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapClampMode );

    switch (internalFormat) {
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16_ARB:
	case GL_DEPTH_COMPONENT24_ARB:
	case GL_DEPTH_COMPONENT32_ARB:
		// Fix for sampling depth buffer on old nVidia cards.
		// from http://www.idevgames.com/forums/thread-4141-post-34844.html#pid34844
		nglTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
		nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	default:
		nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
		nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
		break;
	};

    nglBindTexture( GL_TEXTURE_2D, 0 );

    GL_CheckErrors();

	hash = generateHashValue( name );
	image->m_pNext = hashTable[hash];
	hashTable[hash] = image;

    return image;
}

GDR_EXPORT void LoadImageFile(const char *name, byte **pic, uint32_t *width, uint32_t *height, int32_t *channels)
{
	uint64_t size;
	byte *out;
	union {
		void *v;
		byte *b;
	} buffer;

	//
	// load the file
	//
	size = ri.FS_LoadFile(name, &buffer.v);
	if (!buffer.b || size == 0) {
		return;
	}

	out = stbi_load_from_memory(buffer.b, size, (int *)width, (int *)height, channels, 0);
	if (!out) {
		ri.FS_FreeFile(buffer.b);
		ri.Printf(PRINT_DEVELOPER, "LoadImageFile: stbi_load_from_memory(%s) failed, failure reason: %s\n", name, stbi_failure_reason());
		return;
	}

	ri.FS_FreeFile(buffer.v);

	*pic = out;
}

/*
=================
R_LoadImage

Loads any of the supported image types into a canonical
32 bit format.
=================
*/
GDR_EXPORT void R_LoadImage( const char *name, byte **pic, uint32_t *width, uint32_t *height, GLenum *picFormat )
{
	char localName[ MAX_GDR_PATH ];
	const char *ext;
	const char *altName;
	int32_t channels;

	*pic = NULL;
	*width = 0;
	*height = 0;
	*picFormat = GL_RGBA8;

	N_strncpyz( localName, name, sizeof( localName ) );

	ext = COM_GetExtension( localName );

	// If compressed textures are enabled, try loading a DDS first, it'll load fastest
	if (r_arb_texture_compression->i)
	{
		char ddsName[MAX_GDR_PATH];

		COM_StripExtension(name, ddsName, MAX_GDR_PATH);
		N_strcat(ddsName, MAX_GDR_PATH, ".dds");

		// not for now...
//		R_LoadDDS(ddsName, pic, width, height, picFormat, numMips);

		// If loaded, we're done.
		if (*pic)
			return;
	}

	// now we just use stb_image
	LoadImageFile(name, pic, width, height, &channels);
	if (channels == 3)
		*picFormat = GL_RGB8;
}

GDR_EXPORT void CTexture::Shutdown( void ) {
    nglDeleteTextures( 1, &m_Id );
}

/*
===============
R_FindImageFile

Finds or loads the given image.
Returns NULL if it fails, not a default image.
==============
*/
GDR_EXPORT CTexture *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags )
{
	CTexture *image;
	uint32_t width, height;
	byte *pic;
	GLenum picFormat;
	uint64_t hash;
	imgFlags_t checkFlagsTrue, checkFlagsFalse;

	if (!name) {
		return NULL;
	}

	hash = generateHashValue(name);

	//
	// see if the image is already loaded
	//
	for (image=hashTable[hash]; image; image=image->GetNext()) {
		if ( !strcmp( name, image->GetName() ) ) {
			// the white image can be used with any set of parms, but other mismatches are errors
			if ( strcmp( name, "*white" ) ) {
				if ( image->GetFlags() != flags ) {
					ri.Printf( PRINT_DEVELOPER, "WARNING: reused image %s with mixed flags (%i vs %i)\n", name, image->GetFlags(), flags );
				}
			}
			return image;
		}
	}

	//
	// load the pic from disk
	//
	R_LoadImage( name, &pic, &width, &height, &picFormat );
	if ( pic == NULL ) {
		return NULL;
	}
/*
	checkFlagsTrue = IMGFLAG_PICMIP | IMGFLAG_MIPMAP | IMGFLAG_GENNORMALMAP;
	checkFlagsFalse = IMGFLAG_CUBEMAP;
	if (r_normalMapping->i && (picFormat == GL_RGBA8) && (type == IMGTYPE_COLORALPHA) &&
		((flags & checkFlagsTrue) == checkFlagsTrue) && !(flags & checkFlagsFalse))
	{
		char normalName[MAX_GDR_PATH];
		CTexture *normalImage;
		uint32_t normalWidth, normalHeight;
		imgFlags_t normalFlags;

		normalFlags = (flags & ~IMGFLAG_GENNORMALMAP) | IMGFLAG_NOLIGHTSCALE;

		COM_StripExtension(name, normalName, MAX_GDR_PATH);
		N_strcat(normalName, MAX_GDR_PATH, "_n");

		// find normalmap in case it's there
		normalImage = R_FindImageFile(normalName, IMGTYPE_NORMAL, normalFlags);

		// if not, generate it
		if (normalImage == NULL)
		{
			byte *normalPic;
			uint32_t x, y;

			normalWidth = width;
			normalHeight = height;
			normalPic = ri.Malloc(width * height * 4);
			RGBAtoNormal(pic, normalPic, width, height, flags & IMGFLAG_CLAMPTOEDGE);

#if 1
			// Brighten up the original image to work with the normal map
			RGBAtoYCoCgA(pic, pic, width, height);
			for (y = 0; y < height; y++)
			{
				byte *picbyte  = pic       + y * width * 4;
				byte *normbyte = normalPic + y * width * 4;
				for (x = 0; x < width; x++)
				{
					uint32_t div = MAX(normbyte[2] - 127, 16);
					picbyte[0] = CLAMP(picbyte[0] * 128 / div, 0, 255);
					picbyte  += 4;
					normbyte += 4;
				}
			}
			YCoCgAtoRGBA(pic, pic, width, height);
#else
			// Blur original image's luma to work with the normal map
			{
				byte *blurPic;

				RGBAtoYCoCgA(pic, pic, width, height);
				blurPic = ri.Malloc(width * height);

				for (y = 1; y < height - 1; y++)
				{
					byte *picbyte  = pic     + y * width * 4;
					byte *blurbyte = blurPic + y * width;

					picbyte += 4;
					blurbyte += 1;

					for (x = 1; x < width - 1; x++)
					{
						int result;

						result = *(picbyte - (width + 1) * 4) + *(picbyte - width * 4) + *(picbyte - (width - 1) * 4) +
						         *(picbyte -          1  * 4) + *(picbyte            ) + *(picbyte +          1  * 4) +
						         *(picbyte + (width - 1) * 4) + *(picbyte + width * 4) + *(picbyte + (width + 1) * 4);

						result /= 9;

						*blurbyte = result;
						picbyte += 4;
						blurbyte += 1;
					}
				}

				// FIXME: do borders

				for (y = 1; y < height - 1; y++)
				{
					byte *picbyte  = pic     + y * width * 4;
					byte *blurbyte = blurPic + y * width;

					picbyte += 4;
					blurbyte += 1;

					for (x = 1; x < width - 1; x++)
					{
						picbyte[0] = *blurbyte;
						picbyte += 4;
						blurbyte += 1;
					}
				}

				ri.Free(blurPic);

				YCoCgAtoRGBA(pic, pic, width, height);
			}
#endif

			R_CreateImage( normalName, normalPic, normalWidth, normalHeight, IMGTYPE_NORMAL, normalFlags, GL_RGBA, picFormat);
			ri.Free( normalPic );	
		}
	}

	// force mipmaps off if image is compressed but doesn't have enough mips
	if ((flags & IMGFLAG_MIPMAP) && picFormat != GL_RGBA8 && picFormat != GL_SRGB8_ALPHA8_EXT)
	{
		uint32_t wh = MAX(width, height);
		uint32_t neededMips = 0;
		while (wh)
		{
			neededMips++;
			wh >>= 1;
		}
		if (neededMips > picNumMips)
			flags &= ~IMGFLAG_MIPMAP;
	}
    */

	image = CTexture::CreateImage( ( char * ) name, pic, width, height, type, flags, GL_RGBA, picFormat );
	free( pic ); // not in the zone heap
	return image;
}

GDR_EXPORT void R_InitTextures( void )
{
    memset(hashTable, 0, sizeof(hashTable));
	switch (r_textureFiltering->i) {
	case TexFilter_Linear:
		gl_filter_max = GL_LINEAR;
		gl_filter_min = GL_LINEAR;
		break;
	case TexFilter_Nearest:
		gl_filter_max = GL_NEAREST;
		gl_filter_min = GL_NEAREST;
		break;
	case TexFilter_Bilinear:
		gl_filter_max = GL_LINEAR;
		gl_filter_min = GL_NEAREST;
		break;
	case TexFilter_Trilinear:
		gl_filter_max = GL_NEAREST;
		gl_filter_min = GL_LINEAR;
		break;
	};
}

GDR_EXPORT void R_DeleteTextures( void ) {
    for (auto *it : rg.textures) {
        it->Shutdown();
    }

    memset( rg.textures, 0, sizeof(rg.textures) );
    rg.numTextures = 0;
}
