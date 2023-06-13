#include "n_shared.h"
#include "n_scf.h"
#include "g_bff.h"
#include "m_renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_FILE_HASH 1024
static texture_t* textures[MAX_FILE_HASH];

typedef struct
{
    const char *str;
    GLint value;
} texmode_t;

typedef struct
{
    const char *str;
    uint16_t type;
} textype_t;

static const textype_t types[] = {
    {"jpg", TEX_JPG},
    {"png", TEX_PNG},
    {"tga", TEX_TGA},
    {"bmp", TEX_BMP},
};

static const texmode_t modes[] = {
    {"GL_NEAREST", GL_NEAREST},
    {"GL_LINEAR", GL_LINEAR},
    {"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR},
    {"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST},
    {"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR},
    {"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST},
};

static GLint R_TexMagFilter(void)
{
    GLint filter = -1;
    for (uint32_t i = 0; i < arraylen(modes); i++) {
        if (N_strcmp(modes[i].str, r_texture_magfilter.s)) {
            filter = modes[i].value;
            break;
        }
    }
    if (filter == -1) {
        Con_Printf("WARNING: r_texture_magfilter was invalid, using default of GL_NEAREST");
        filter = GL_NEAREST;
        N_strcpy(r_texture_magfilter.s, "GL_NEAREST");
    }
    return filter;
}
static GLint R_TexMinFilter(void)
{
    GLint filter = -1;
    for (uint32_t i = 0; i < arraylen(modes); i++) {
        if (N_strcmp(modes[i].str, r_texture_minfilter.s)) {
            filter = modes[i].value;
            break;
        }
    }
    if (filter == -1) {
        Con_Printf("WARNING: r_texture_minfilter was invalid, using default of GL_LINEAR_MIPMAP_LINEAR");
        filter = GL_LINEAR_MIPMAP_LINEAR;
        N_strcpy(r_texture_minfilter.s, "GL_LINEAR_MIPMAP_LINEAR");
    }
    return filter;
}

void R_UpdateTextures(void)
{
    // clear the texture bound, if there is any
    glBindTexture(GL_TEXTURE_2D, 0);

    for (uint32_t i = 0; i < renderer->numTextures; i++) {
        glBindTexture(GL_TEXTURE_2D, renderer->textures[i]->id);

        texture_t* tex = renderer->textures[i];
        tex->minFilter = R_TexMinFilter();
        tex->magFilter = R_TexMagFilter();
        tex->wrapS = GL_REPEAT;
        tex->wrapT = GL_REPEAT;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->wrapS);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void I_CacheTextures(bffinfo_t *info)
{
    for (uint32_t i = 0; i < info->numTextures; i++) {
        renderer->textures[i] = R_CreateTexture("temptex", info->textures[i].name, (const void *)&info->textures[i]);
    }
    renderer->numTextures = info->numTextures;
}

texture_t* R_GetTexture(const char *name)
{
    return textures[Com_GenerateHashValue(name, MAX_TEXTURES)];
}

/*
NOTE TO SELF: for some reason, bffs don't work well with any textures that have very low (16x16 ish) resolution, just keep that in mind,
otherwise, it'll cause a crash.
*/
texture_t* R_CreateTexture(const char *filepath, const char *name, const void *texture)
{
    const bfftexture_t *bfftex = (const bfftexture_t *)texture;
    texture_t* tex = (texture_t *)Hunk_Alloc(sizeof(texture_t), name, h_low);

    tex->minFilter = R_TexMinFilter();
    tex->magFilter = R_TexMagFilter();
    tex->wrapS = GL_REPEAT;
    tex->wrapT = GL_REPEAT;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->wrapT);

    stbi_uc *image = stbi_load_from_memory((const stbi_uc *)bfftex->fileBuffer, bfftex->fileSize, &tex->width, &tex->height, &tex->channels, 4);
    if (!image) {
        N_Error("R_CreateTexture: stbi_load_from_memory failed to load file %s, error string: %s", name, stbi_failure_reason());
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    tex->data = (byte *)Hunk_Alloc(tex->width * tex->height * 4, "texbuffer", h_low);
    memcpy(tex->data, image, tex->width * tex->height * 4);
    free(image);

    Con_Printf("Loaded texture file %s", filepath);

    uint64_t hash = Com_GenerateHashValue(name, MAX_TEXTURES);
    textures[hash] = tex;

    return tex;
}

#if 0
typedef struct
{
	char id[2];
	unsigned fileSize;
	unsigned reserved0;
	unsigned bitmapDataOffset;
	unsigned bitmapHeaderSize;
	unsigned width;
	unsigned height;
	unsigned short planes;
	unsigned short bitsPerPixel;
	unsigned compression;
	unsigned bitmapDataSize;
	unsigned hRes;
	unsigned vRes;
	unsigned colors;
	unsigned importantColors;
	unsigned char palette[256][4];
} BMPHeader_t;

static void R_LoadBMP(const char *name, byte **pic, uint32_t *width, uint32_t *height)
{
	uint32_t columns, rows;
	unsigned numPixels;
	byte *pixbuf;
	uint32_t row, column;
	byte *buf_p;
	byte *end;
	
    union {
		byte *b;
		void *v;
	} buffer;
	
    uint32_t length;
	BMPHeader_t bmpHeader;
	byte *bmpRGBA;

	*pic = NULL;

	if(width)
		*width = 0;

	if(height)
		*height = 0;

	//
	// load the file
	//
    length = FS_LoadFile(name, &buffer.v);
	if (!buffer.b || length < 0) {
		return;
	}

	if (length < 54) {
        N_Error("LoadBMP: header too short (%s)", name );
	}

	buf_p = buffer.b;
	end = buffer.b + length;

	bmpHeader.id[0] = *buf_p++;
	bmpHeader.id[1] = *buf_p++;
	bmpHeader.fileSize = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.reserved0 = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataOffset = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapHeaderSize = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.width = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.height = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.planes = LittleShort( * ( short * ) buf_p );
	buf_p += 2;
	bmpHeader.bitsPerPixel = LittleShort( * ( short * ) buf_p );
	buf_p += 2;
	bmpHeader.compression = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataSize = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.hRes = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.vRes = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.colors = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;
	bmpHeader.importantColors = SDL_SwapLE32( * ( int * ) buf_p );
	buf_p += 4;

	if (bmpHeader.bitsPerPixel == 8) {
		if (buf_p + sizeof(bmpHeader.palette) > end)
			N_Error( "LoadBMP: header too short (%s)", name );

		memcpy( bmpHeader.palette, buf_p, sizeof( bmpHeader.palette ) );
	}

	if (buffer.b + bmpHeader.bitmapDataOffset > end) {
		N_Error( "LoadBMP: invalid offset value in header (%s)", name );
	}

	buf_p = buffer.b + bmpHeader.bitmapDataOffset;

	if (bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M')  {
		ri.Error( ERR_DROP, "LoadBMP: only Windows-style BMP files supported (%s)", name );
	}
	if ( bmpHeader.fileSize != (unsigned int)length )
	{
		ri.Error( ERR_DROP, "LoadBMP: header size does not match file size (%u vs. %u) (%s)", bmpHeader.fileSize, length, name );
	}
	if ( bmpHeader.compression != 0 )
	{
		ri.Error( ERR_DROP, "LoadBMP: only uncompressed BMP files supported (%s)", name );
	}
	if ( bmpHeader.bitsPerPixel < 8 )
	{
		ri.Error( ERR_DROP, "LoadBMP: monochrome and 4-bit BMP files not supported (%s)", name );
	}

	switch ( bmpHeader.bitsPerPixel )
	{
		case 8:
		case 16:
		case 24:
		case 32:
			break;
		default:
			ri.Error( ERR_DROP, "LoadBMP: illegal pixel_size '%hu' in file '%s'", bmpHeader.bitsPerPixel, name );
			break;
	}

	columns = bmpHeader.width;
	rows = bmpHeader.height;
	if ( rows < 0 )
		rows = -rows;
	numPixels = columns * rows;

	if(columns <= 0 || !rows || numPixels > 0x1FFFFFFF // 4*1FFFFFFF == 0x7FFFFFFC < 0x7FFFFFFF
	    || ((numPixels * 4) / columns) / 4 != (unsigned int)rows)
	{
	  ri.Error (ERR_DROP, "LoadBMP: %s has an invalid image size", name);
	}
	if(buf_p + numPixels*bmpHeader.bitsPerPixel/8 > end)
	{
	  ri.Error (ERR_DROP, "LoadBMP: file truncated (%s)", name);
	}

	if ( width ) 
		*width = columns;
	if ( height )
		*height = rows;

	bmpRGBA = ri.Malloc( numPixels * 4 );
	*pic = bmpRGBA;


	for ( row = rows-1; row >= 0; row-- )
	{
		pixbuf = bmpRGBA + row*columns*4;

		for ( column = 0; column < columns; column++ )
		{
			unsigned char red, green, blue, alpha;
			int palIndex;
			unsigned short shortPixel;

			switch ( bmpHeader.bitsPerPixel )
			{
			case 8:
				palIndex = *buf_p++;
				*pixbuf++ = bmpHeader.palette[palIndex][2];
				*pixbuf++ = bmpHeader.palette[palIndex][1];
				*pixbuf++ = bmpHeader.palette[palIndex][0];
				*pixbuf++ = 0xff;
				break;
			case 16:
				shortPixel = * ( unsigned short * ) pixbuf;
				pixbuf += 2;
				*pixbuf++ = ( shortPixel & ( 31 << 10 ) ) >> 7;
				*pixbuf++ = ( shortPixel & ( 31 << 5 ) ) >> 2;
				*pixbuf++ = ( shortPixel & ( 31 ) ) << 3;
				*pixbuf++ = 0xff;
				break;

			case 24:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = 255;
				break;
			case 32:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				alpha = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = alpha;
				break;
			}
		}
	}

    FS_FClose(b);
	ri.FS_FreeFile( buffer.v );
}

#endif