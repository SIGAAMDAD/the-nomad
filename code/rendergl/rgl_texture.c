#include "rgl_local.h"
#include <ctype.h>

static void *Image_Malloc( size_t size ) {
	return ri.Malloc( size );
}
static void *Image_Realloc( void *ptr, size_t nsize ) {
	return ri.Realloc( ptr, nsize );
}
static void Image_Free( void *ptr ) {
	if ( ptr ) {
		ri.Free( ptr );
	}
}

#define STBI_MALLOC(size) Image_Malloc(size)
#define STBI_REALLOC(ptr,nsize) Image_Realloc(ptr,nsize)
#define STBI_FREE(ptr) Image_Free(ptr)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static byte s_intensitytable[256];
static unsigned char s_gammatable[256];

int gl_filter_min = GL_LINEAR;
int gl_filter_max = GL_NEAREST;

#define FILE_HASH_SIZE MAX_RENDER_TEXTURES
static texture_t *hashTable[FILE_HASH_SIZE];

/*
** R_GammaCorrect
*/
void R_GammaCorrect( byte *buffer, uint64_t bufSize ) {
	uint64_t i;
	ri.Printf( PRINT_DEVELOPER, "----- R_GammaCorrect ------" );

	for ( i = 0; i < bufSize; i++ ) {
		buffer[i] = s_gammatable[buffer[i]];
	}
}

typedef struct {
    const char *filter;
    int min, mag;
} textureFilterMode_t;

static const textureFilterMode_t filters[] = {
    { "Bilinear", GL_LINEAR, GL_LINEAR },
    { "Nearest", GL_NEAREST, GL_NEAREST },
    { "Linear Nearest", GL_LINEAR, GL_NEAREST },
    { "Nearest Linear", GL_NEAREST, GL_LINEAR }
};

/*
================
return a hash f for the filename
================
*/
static uint64_t generateHashValue( const char *fname )
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
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

void R_UpdateTextures( void )
{
    uint32_t i;
    texture_t *t;

    for ( i = 0; i < arraylen( filters ); i++ ) {
        if ( !N_stricmp( filters[i].filter, r_textureMode->s ) ) {
            break;
        }
    }

    if ( i == arraylen( filters ) ) {
        ri.Cvar_Reset( "r_textureMode" );
        return;
    }

	ri.Printf( PRINT_INFO, "Updating textures...\n" );

	gl_filter_min = filters[i].min;
	gl_filter_max = filters[i].mag;

    // change all the texture filters
    for ( i = 0; i < rg.numTextures; i++ ) {
        t = rg.textures[i];

	    GL_BindTexture( TB_COLORMAP, t );

	    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
	    nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );

		if ( r_arb_texture_filter_anisotropic->i ) {
			// update the anisotropy while we're at it
			nglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, r_arb_texture_max_anisotropy->f );
		}

	    GL_BindTexture( TB_COLORMAP, 0 );
    }
}

/*
===============
R_SumOfUsedImages
===============
*/
uint64_t R_SumOfUsedImages( void )
{
	uint64_t total;
	uint32_t i;

	total = 0;
	for ( i = 0; i < rg.numTextures; i++ ) {
		if ( rg.textures[i]->frameUsed == rg.frameCount) {
			total += rg.textures[i]->uploadWidth * rg.textures[i]->uploadHeight;
		}
	}

	return total;
}


/*
===============
R_ImageList_f
===============
*/
void R_ImageList_f( void )
{
	uint32_t i;

	ri.Printf( PRINT_INFO, "\n     --w-- --h-- -type-- -size- --name-------\n" );

	glState.memstats.numTextures = 0;
	glState.memstats.estTextureMemUsed = 0;
	for ( i = 0 ; i < rg.numTextures ; i++ ) {
		texture_t *image = rg.textures[i];
		const char *format = "????   ";
		const char *sizeSuffix;
		uint32_t displaySize;
		uint32_t estSize = 0;

		estSize = image->uploadWidth * image->uploadHeight;

		switch ( image->internalFormat ) {
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			format = "sDXT1  ";
			// 64 bits per 16 pixels, so 4 bits per pixel
			estSize /= 2;
			break;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			format = "sDXT5  ";
			// 128 bits per 16 pixels, so 1 byte per pixel
			break;
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
			format = "sBPTC  ";
			// 128 bits per 16 pixels, so 1 byte per pixel
			break;
		case GL_COMPRESSED_RG_RGTC2:
			format = "RGTC2  ";
			// 128 bits per 16 pixels, so 1 byte per pixel
			break;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			format = "DXT1   ";
			// 64 bits per 16 pixels, so 4 bits per pixel
			estSize /= 2;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			format = "DXT1a  ";
			// 64 bits per 16 pixels, so 4 bits per pixel
			estSize /= 2;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			format = "DXT5   ";
			// 128 bits per 16 pixels, so 1 byte per pixel
			break;
		case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
			format = "BPTC   ";
			// 128 bits per 16 pixels, so 1 byte per pixel
			break;
		case GL_RGB4_S3TC:
			format = "S3TC   ";
			// same as DXT1?
			estSize /= 2;
			break;
		case GL_RGBA16F_ARB:
			format = "RGBA16F";
			// 8 bytes per pixel
			estSize *= 16;
			break;
		case GL_RGBA16:
			format = "RGBA16 ";
			// 8 bytes per pixel
			estSize *= 8;
			break;
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGBA8:
		case GL_RGBA12:
		case GL_RGBA:
		case GL_RGB12:
			format = "RGBA   ";
			// 4 bytes per pixel
			estSize *= 4;
			break;
		case GL_RGBA32F_ARB:
		case GL_RGB32F_ARB:
			format = "RGBA32F ";
			// 16 bytes per pixel?
			estSize *= 16;
			break;
		case GL_LUMINANCE8:
		case GL_LUMINANCE:
			format = "L      ";
			// 1 byte per pixel?
			break;
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB2_EXT:
		case GL_RGB:
			format = "RGB    ";
			// 3 bytes per pixel?
			estSize *= 3;
			break;
		case GL_LUMINANCE8_ALPHA8:
		case GL_LUMINANCE_ALPHA:
			format = "LA     ";
			// 2 bytes per pixel?
			estSize *= 2;
			break;
		case GL_SRGB_EXT:
		case GL_SRGB8_EXT:
			format = "sRGB   ";
			// 3 bytes per pixel?
			estSize *= 3;
			break;
		case GL_SRGB_ALPHA_EXT:
		case GL_SRGB8_ALPHA8_EXT:
			format = "sRGBA  ";
			// 4 bytes per pixel?
			estSize *= 4;
			break;
		case GL_SLUMINANCE_EXT:
		case GL_SLUMINANCE8_EXT:
			format = "sL     ";
			// 1 byte per pixel?
			break;
		case GL_SLUMINANCE_ALPHA_EXT:
		case GL_SLUMINANCE8_ALPHA8_EXT:
			format = "sLA    ";
			// 2 byte per pixel?
			estSize *= 2;
			break;
		case GL_DEPTH_COMPONENT16:
			format = "Depth16";
			// 2 bytes per pixel
			estSize *= 2;
			break;
		case GL_DEPTH_COMPONENT24:
			format = "Depth24";
			// 3 bytes per pixel
			estSize *= 3;
			break;
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT32:
			format = "Depth32";
			// 4 bytes per pixel
			estSize *= 4;
			break;
		default:
			ri.Printf( PRINT_WARNING, "unknown texture format 0x%04i\n", image->internalFormat );
		};

		// mipmap adds about 50%
		if (image->flags & IMGFLAG_MIPMAP)
			estSize += estSize / 2;

		sizeSuffix = "b ";
		displaySize = estSize;

		if ( displaySize > 1024 ) {
			displaySize /= 1024;
			sizeSuffix = "kb";
		}
		if ( displaySize > 1024 ) {
			displaySize /= 1024;
			sizeSuffix = "Mb";
		}
		if ( displaySize > 1024 ) {
			displaySize /= 1024;
			sizeSuffix = "Gb";
		}

		ri.Printf(PRINT_INFO, "%u:   %5u %5u %s %4u%s %s\n", i, image->width, image->height, format, displaySize, sizeSuffix, image->imgName);
		glState.memstats.estTextureMemUsed += estSize;
		glState.memstats.numTextures++;
	}

	ri.Printf( PRINT_INFO, " ---------\n" );
	ri.Printf( PRINT_INFO, " approx %u bytes\n", glState.memstats.estTextureMemUsed );
	ri.Printf( PRINT_INFO, " %u total images\n\n", rg.numTextures );
}

//=======================================================================


/*
================
ResampleTexture

Used to resample images in a more general than quartering fashion.

This will only be filtered properly if the resampled size
is greater than half the original size.

If a larger shrinking is needed, use the mipmap function 
before or after.
================
*/
static void ResampleTexture( byte *in, uint32_t inwidth, uint32_t inheight, byte *out,  
							uint32_t outwidth, uint32_t outheight ) {
	uint32_t i, j;
	byte	*inrow, *inrow2;
	uint32_t frac, fracstep;
	uint32_t p1[2048], p2[2048];
	byte	*pix1, *pix2, *pix3, *pix4;

	if (outwidth>2048)
		ri.Error(ERR_DROP, "ResampleTexture: max width");
								
	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for ( i=0 ; i<outwidth ; i++ ) {
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for ( i=0 ; i<outwidth ; i++ ) {
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++) {
		inrow = in + 4*inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + 4*inwidth*(int)((i+0.75)*inheight/outheight);
		for (j=0 ; j<outwidth ; j++) {
			pix1 = inrow + p1[j];
			pix2 = inrow + p2[j];
			pix3 = inrow2 + p1[j];
			pix4 = inrow2 + p2[j];
			*out++ = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			*out++ = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			*out++ = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			*out++ = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}

static void RGBAtoYCoCgA(const byte *in, byte *out, int width, int height)
{
	int x, y;

	for (y = 0; y < height; y++)
	{
		const byte *inbyte  = in  + y * width * 4;
		byte       *outbyte = out + y * width * 4;

		for (x = 0; x < width; x++)
		{
			byte r, g, b, a, rb2;

			r = *inbyte++;
			g = *inbyte++;
			b = *inbyte++;
			a = *inbyte++;
			rb2 = (r + b) >> 1;

			*outbyte++ = (g + rb2) >> 1;       // Y  =  R/4 + G/2 + B/4
			*outbyte++ = (r - b + 256) >> 1;   // Co =  R/2       - B/2
			*outbyte++ = (g - rb2 + 256) >> 1; // Cg = -R/4 + G/2 - B/4
			*outbyte++ = a;
		}
	}
}

static void YCoCgAtoRGBA(const byte *in, byte *out, int width, int height)
{
	int x, y;

	for (y = 0; y < height; y++)
	{
		const byte *inbyte  = in  + y * width * 4;
		byte       *outbyte = out + y * width * 4;

		for (x = 0; x < width; x++)
		{
			byte _Y, Co, Cg, a;

			_Y = *inbyte++;
			Co = *inbyte++;
			Cg = *inbyte++;
			a  = *inbyte++;

			*outbyte++ = CLAMP(_Y + Co - Cg,       0, 255); // R = Y + Co - Cg
			*outbyte++ = CLAMP(_Y      + Cg - 128, 0, 255); // G = Y + Cg
			*outbyte++ = CLAMP(_Y - Co - Cg + 256, 0, 255); // B = Y - Co - Cg
			*outbyte++ = a;
		}
	}
}


// uses a sobel filter to change a texture to a normal map
static void RGBAtoNormal(const byte *in, byte *out, uint32_t width, uint32_t height, qboolean clampToEdge)
{
	int x, y, max;

	// convert to heightmap, storing in alpha
	// same as converting to Y in YCoCg
	max = 1;
	for (y = 0; y < height; y++) {
		const byte *inbyte  = in  + y * width * 4;
		byte       *outbyte = out + y * width * 4 + 3;

		for (x = 0; x < width; x++) {
			byte result = (inbyte[0] >> 2) + (inbyte[1] >> 1) + (inbyte[2] >> 2);
			result = result * result / 255; // Make linear
			*outbyte = result;
			max = MAX(max, *outbyte);
			outbyte += 4;
			inbyte  += 4;
		}
	}

	// level out heights
	if (max < 255) {
		for (y = 0; y < height; y++) {
			byte *outbyte = out + y * width * 4 + 3;

			for (x = 0; x < width; x++) {
				*outbyte = *outbyte + (255 - max);
				outbyte += 4;
			}
		}
	}


	// now run sobel filter over height fs to generate X and Y
	// then normalize
	for (y = 0; y < height; y++) {
		byte *outbyte = out + y * width * 4;

		for (x = 0; x < width; x++) {
			// 0 1 2
			// 3 4 5
			// 6 7 8

			byte s[9];
			uint32_t x2, y2, i;
			vec3_t normal;

			i = 0;
			for (y2 = -1; y2 <= 1; y2++) {
				uint32_t src_y = y + y2;

				if (clampToEdge) {
					src_y = CLAMP(src_y, 0, height - 1);
				}
				else {
					src_y = (src_y + height) % height;
				}


				for (x2 = -1; x2 <= 1; x2++) {
					uint32_t src_x = x + x2;

					if (clampToEdge) {
						src_x = CLAMP(src_x, 0, width - 1);
					}
					else {
						src_x = (src_x + width) % width;
					}

					s[i++] = *(out + (src_y * width + src_x) * 4 + 3);
				}
			}

			normal[0] =        s[0]            -     s[2]
						 + 2 * s[3]            - 2 * s[5]
						 +     s[6]            -     s[8];

			normal[1] =        s[0] + 2 * s[1] +     s[2]

						 -     s[6] - 2 * s[7] -     s[8];

			normal[2] = s[4] * 4;

			if (!VectorNormalize2(normal, normal)) {
				VectorSet(normal, 0, 0, 1);
			}

			*outbyte++ = FloatToOffsetByte(normal[0]);
			*outbyte++ = FloatToOffsetByte(normal[1]);
			*outbyte++ = FloatToOffsetByte(normal[2]);
			outbyte++;
		}
	}
}

#define COPYSAMPLE(a,b) *(unsigned int *)(a) = *(unsigned int *)(b)

// based on Fast Curve Based Interpolation
// from Fast Artifacts-Free Image Interpolation (http://www.andreagiachetti.it/icbi/)
// assumes data has a 2 pixel thick border of clamped or wrapped data
// expects data to be a grid with even (0, 0), (2, 0), (0, 2), (2, 2) etc pixels filled
// only performs FCBI on specified component
static void DoFCBI(byte *in, byte *out, int width, int height, int component)
{
	int x, y;
	byte *outbyte, *inbyte;

	// copy in to out
	for (y = 2; y < height - 2; y += 2)
	{
		inbyte  = in  + (y * width + 2) * 4 + component;
		outbyte = out + (y * width + 2) * 4 + component;

		for (x = 2; x < width - 2; x += 2)
		{
			*outbyte = *inbyte;
			outbyte += 8;
			inbyte += 8;
		}
	}
	
	for (y = 3; y < height - 3; y += 2)
	{
		// diagonals
		//
		// NWp  - northwest interpolated pixel
		// NEp  - northeast interpolated pixel
		// NWd  - northwest first derivative
		// NEd  - northeast first derivative
		// NWdd - northwest second derivative
		// NEdd - northeast second derivative
		//
		// Uses these samples:
		//
		//         0
		//   - - a - b - -
		//   - - - - - - -
		//   c - d - e - f
		// 0 - - - - - - -
		//   g - h - i - j
		//   - - - - - - -
		//   - - k - l - -
		//
		// x+2 uses these samples:
		//
		//         0
		//   - - - - a - b - -
		//   - - - - - - - - -
		//   - - c - d - e - f
		// 0 - - - - - - - - -
		//   - - g - h - i - j
		//   - - - - - - - - -
		//   - - - - k - l - -
		//
		// so we can reuse 8 of them on next iteration
		//
		// a=b, c=d, d=e, e=f, g=h, h=i, i=j, k=l
		//
		// only b, f, j, and l need to be sampled on next iteration

		byte sa, sb, sc, sd, se, sf, sg, sh, si, sj, sk, sl;
		byte *line1, *line2, *line3, *line4;

		x = 3;

		// optimization one
		//                       SAMPLE2(sa, x-1, y-3);
		//SAMPLE2(sc, x-3, y-1); SAMPLE2(sd, x-1, y-1); SAMPLE2(se, x+1, y-1);
		//SAMPLE2(sg, x-3, y+1); SAMPLE2(sh, x-1, y+1); SAMPLE2(si, x+1, y+1);
		//                       SAMPLE2(sk, x-1, y+3);

		// optimization two
		line1 = in + ((y - 3) * width + (x - 1)) * 4 + component;
		line2 = in + ((y - 1) * width + (x - 3)) * 4 + component;
		line3 = in + ((y + 1) * width + (x - 3)) * 4 + component;
		line4 = in + ((y + 3) * width + (x - 1)) * 4 + component;

		//                                   COPYSAMPLE(sa, line1); line1 += 8;
		//COPYSAMPLE(sc, line2); line2 += 8; COPYSAMPLE(sd, line2); line2 += 8; COPYSAMPLE(se, line2); line2 += 8;
		//COPYSAMPLE(sg, line3); line3 += 8; COPYSAMPLE(sh, line3); line3 += 8; COPYSAMPLE(si, line3); line3 += 8;
		//                                   COPYSAMPLE(sk, line4); line4 += 8;

		                         sa = *line1; line1 += 8;
		sc = *line2; line2 += 8; sd = *line2; line2 += 8; se = *line2; line2 += 8;
		sg = *line3; line3 += 8; sh = *line3; line3 += 8; si = *line3; line3 += 8;
		                         sk = *line4; line4 += 8;

		outbyte = out + (y * width + x) * 4 + component;

		for ( ; x < width - 3; x += 2)
		{
			int NWd, NEd, NWp, NEp;

			// original
			//                       SAMPLE2(sa, x-1, y-3); SAMPLE2(sb, x+1, y-3);
			//SAMPLE2(sc, x-3, y-1); SAMPLE2(sd, x-1, y-1); SAMPLE2(se, x+1, y-1); SAMPLE2(sf, x+3, y-1);
			//SAMPLE2(sg, x-3, y+1); SAMPLE2(sh, x-1, y+1); SAMPLE2(si, x+1, y+1); SAMPLE2(sj, x+3, y+1);
			//                       SAMPLE2(sk, x-1, y+3); SAMPLE2(sl, x+1, y+3);

			// optimization one
			//SAMPLE2(sb, x+1, y-3);
			//SAMPLE2(sf, x+3, y-1);
			//SAMPLE2(sj, x+3, y+1);
			//SAMPLE2(sl, x+1, y+3);

			// optimization two
			//COPYSAMPLE(sb, line1); line1 += 8;
			//COPYSAMPLE(sf, line2); line2 += 8;
			//COPYSAMPLE(sj, line3); line3 += 8;
			//COPYSAMPLE(sl, line4); line4 += 8;

			sb = *line1; line1 += 8;
			sf = *line2; line2 += 8;
			sj = *line3; line3 += 8;
			sl = *line4; line4 += 8;

			NWp = sd + si;
			NEp = se + sh;
			NWd = abs(sd - si);
			NEd = abs(se - sh);

			if (NWd > 100 || NEd > 100 || abs(NWp-NEp) > 200)
			{
				if (NWd < NEd)
					*outbyte = NWp >> 1;
				else
					*outbyte = NEp >> 1;
			}
			else
			{
				int NWdd, NEdd;

				//NEdd = abs(sg + sd + sb - 3 * (se + sh) + sk + si + sf);
				//NWdd = abs(sa + se + sj - 3 * (sd + si) + sc + sh + sl);
				NEdd = abs(sg + sb - 3 * NEp + sk + sf + NWp);
				NWdd = abs(sa + sj - 3 * NWp + sc + sl + NEp);

				if (NWdd > NEdd)
					*outbyte = NWp >> 1;
				else
					*outbyte = NEp >> 1;
			}

			outbyte += 8;

			//                    COPYSAMPLE(sa, sb);
			//COPYSAMPLE(sc, sd); COPYSAMPLE(sd, se); COPYSAMPLE(se, sf);
			//COPYSAMPLE(sg, sh); COPYSAMPLE(sh, si); COPYSAMPLE(si, sj);
			//                    COPYSAMPLE(sk, sl);

			         sa = sb;
			sc = sd; sd = se; se = sf;
			sg = sh; sh = si; si = sj;
			         sk = sl;
		}
	}

	// hack: copy out to in again
	for (y = 3; y < height - 3; y += 2)
	{
		inbyte = out + (y * width + 3) * 4 + component;
		outbyte = in + (y * width + 3) * 4 + component;

		for (x = 3; x < width - 3; x += 2)
		{
			*outbyte = *inbyte;
			outbyte += 8;
			inbyte += 8;
		}
	}
	
	for (y = 2; y < height - 3; y++)
	{
		// horizontal & vertical
		//
		// hp  - horizontally interpolated pixel
		// vp  - vertically interpolated pixel
		// hd  - horizontal first derivative
		// vd  - vertical first derivative
		// hdd - horizontal second derivative
		// vdd - vertical second derivative
		// Uses these samples:
		//
		//       0
		//   - a - b -
		//   c - d - e
		// 0 - f - g -
		//   h - i - j
		//   - k - l -
		//
		// x+2 uses these samples:
		//
		//       0
		//   - - - a - b -
		//   - - c - d - e
		// 0 - - - f - g -
		//   - - h - i - j
		//   - - - k - l -
		//
		// so we can reuse 7 of them on next iteration
		//
		// a=b, c=d, d=e, f=g, h=i, i=j, k=l
		//
		// only b, e, g, j, and l need to be sampled on next iteration

		byte sa, sb, sc, sd, se, sf, sg, sh, si, sj, sk, sl;
		byte *line1, *line2, *line3, *line4, *line5;

		//x = (y + 1) % 2;
		x = (y + 1) % 2 + 2;
		
		// optimization one
		//            SAMPLE2(sa, x-1, y-2);
		//SAMPLE2(sc, x-2, y-1); SAMPLE2(sd, x,   y-1);
		//            SAMPLE2(sf, x-1, y  );
		//SAMPLE2(sh, x-2, y+1); SAMPLE2(si, x,   y+1);
		//            SAMPLE2(sk, x-1, y+2);

		line1 = in + ((y - 2) * width + (x - 1)) * 4 + component;
		line2 = in + ((y - 1) * width + (x - 2)) * 4 + component;
		line3 = in + ((y    ) * width + (x - 1)) * 4 + component;
		line4 = in + ((y + 1) * width + (x - 2)) * 4 + component;
		line5 = in + ((y + 2) * width + (x - 1)) * 4 + component;

		//                 COPYSAMPLE(sa, line1); line1 += 8;
		//COPYSAMPLE(sc, line2); line2 += 8; COPYSAMPLE(sd, line2); line2 += 8;
		//                 COPYSAMPLE(sf, line3); line3 += 8;
		//COPYSAMPLE(sh, line4); line4 += 8; COPYSAMPLE(si, line4); line4 += 8;
        //                 COPYSAMPLE(sk, line5); line5 += 8;

		             sa = *line1; line1 += 8;
		sc = *line2; line2 += 8; sd = *line2; line2 += 8;
		             sf = *line3; line3 += 8;
		sh = *line4; line4 += 8; si = *line4; line4 += 8;
		             sk = *line5; line5 += 8;

		outbyte = out + (y * width + x) * 4 + component;

		for ( ; x < width - 3; x+=2)
		{
			int hd, vd, hp, vp;

			//            SAMPLE2(sa, x-1, y-2); SAMPLE2(sb, x+1, y-2);
			//SAMPLE2(sc, x-2, y-1); SAMPLE2(sd, x,   y-1); SAMPLE2(se, x+2, y-1);
			//            SAMPLE2(sf, x-1, y  ); SAMPLE2(sg, x+1, y  );
			//SAMPLE2(sh, x-2, y+1); SAMPLE2(si, x,   y+1); SAMPLE2(sj, x+2, y+1);
			//            SAMPLE2(sk, x-1, y+2); SAMPLE2(sl, x+1, y+2);

			// optimization one
			//SAMPLE2(sb, x+1, y-2);
			//SAMPLE2(se, x+2, y-1);
			//SAMPLE2(sg, x+1, y  );
			//SAMPLE2(sj, x+2, y+1);
			//SAMPLE2(sl, x+1, y+2);

			//COPYSAMPLE(sb, line1); line1 += 8;
			//COPYSAMPLE(se, line2); line2 += 8;
			//COPYSAMPLE(sg, line3); line3 += 8;
			//COPYSAMPLE(sj, line4); line4 += 8;
			//COPYSAMPLE(sl, line5); line5 += 8;

			sb = *line1; line1 += 8;
			se = *line2; line2 += 8;
			sg = *line3; line3 += 8;
			sj = *line4; line4 += 8;
			sl = *line5; line5 += 8;

			hp = sf + sg; 
			vp = sd + si;
			hd = abs(sf - sg);
			vd = abs(sd - si);

			if (hd > 100 || vd > 100 || abs(hp-vp) > 200)
			{
				if (hd < vd)
					*outbyte = hp >> 1;
				else
					*outbyte = vp >> 1;
			}
			else
			{
				int hdd, vdd;

				//hdd = abs(sc[i] + sd[i] + se[i] - 3 * (sf[i] + sg[i]) + sh[i] + si[i] + sj[i]);
				//vdd = abs(sa[i] + sf[i] + sk[i] - 3 * (sd[i] + si[i]) + sb[i] + sg[i] + sl[i]);

				hdd = abs(sc + se - 3 * hp + sh + sj + vp);
				vdd = abs(sa + sk - 3 * vp + sb + sl + hp);

				if (hdd > vdd)
					*outbyte = hp >> 1;
				else 
					*outbyte = vp >> 1;
			}

			outbyte += 8;

			//          COPYSAMPLE(sa, sb);
			//COPYSAMPLE(sc, sd); COPYSAMPLE(sd, se);
			//          COPYSAMPLE(sf, sg);
			//COPYSAMPLE(sh, si); COPYSAMPLE(si, sj);
			//          COPYSAMPLE(sk, sl);
			    sa = sb;
			sc = sd; sd = se;
			    sf = sg;
			sh = si; si = sj;
			    sk = sl;
		}
	}
}

// Similar to FCBI, but throws out the second order derivatives for speed
static void DoFCBIQuick(byte *in, byte *out, int width, int height, int component)
{
	int x, y;
	byte *outbyte, *inbyte;

	// copy in to out
	for (y = 2; y < height - 2; y += 2)
	{
		inbyte  = in  + (y * width + 2) * 4 + component;
		outbyte = out + (y * width + 2) * 4 + component;

		for (x = 2; x < width - 2; x += 2)
		{
			*outbyte = *inbyte;
			outbyte += 8;
			inbyte += 8;
		}
	}

	for (y = 3; y < height - 4; y += 2)
	{
		byte sd, se, sh, si;
		byte *line2, *line3;

		x = 3;

		line2 = in + ((y - 1) * width + (x - 1)) * 4 + component;
		line3 = in + ((y + 1) * width + (x - 1)) * 4 + component;

		sd = *line2; line2 += 8;
		sh = *line3; line3 += 8;

		outbyte = out + (y * width + x) * 4 + component;

		for ( ; x < width - 4; x += 2)
		{
			int NWd, NEd, NWp, NEp;

			se = *line2; line2 += 8;
			si = *line3; line3 += 8;

			NWp = sd + si;
			NEp = se + sh;
			NWd = abs(sd - si);
			NEd = abs(se - sh);

			if (NWd < NEd)
				*outbyte = NWp >> 1;
			else
				*outbyte = NEp >> 1;

			outbyte += 8;

			sd = se;
			sh = si;
		}
	}

	// hack: copy out to in again
	for (y = 3; y < height - 3; y += 2)
	{
		inbyte  = out + (y * width + 3) * 4 + component;
		outbyte = in  + (y * width + 3) * 4 + component;

		for (x = 3; x < width - 3; x += 2)
		{
			*outbyte = *inbyte;
			outbyte += 8;
			inbyte += 8;
		}
	}
	
	for (y = 2; y < height - 3; y++)
	{
		byte sd, sf, sg, si;
		byte *line2, *line3, *line4;

		x = (y + 1) % 2 + 2;

		line2 = in + ((y - 1) * width + (x    )) * 4 + component;
		line3 = in + ((y    ) * width + (x - 1)) * 4 + component;
		line4 = in + ((y + 1) * width + (x    )) * 4 + component;

		outbyte = out + (y * width + x) * 4 + component;

		sf = *line3; line3 += 8;

		for ( ; x < width - 3; x+=2)
		{
			int hd, vd, hp, vp;

			sd = *line2; line2 += 8;
			sg = *line3; line3 += 8;
			si = *line4; line4 += 8;
			
			hp = sf + sg; 
			vp = sd + si;
			hd = abs(sf - sg);
			vd = abs(sd - si);

			if (hd < vd)
				*outbyte = hp >> 1;
			else
				*outbyte = vp >> 1;

			outbyte += 8;

			sf = sg;
		}
	}
}

// Similar to DoFCBIQuick, but just takes the average instead of checking derivatives
// as well, this operates on all four components
static void DoLinear(byte *in, byte *out, int width, int height)
{
	int x, y, i;
	byte *outbyte, *inbyte;

	// copy in to out
	for (y = 2; y < height - 2; y += 2)
	{
		x = 2;

		inbyte  = in  + (y * width + x) * 4;
		outbyte = out + (y * width + x) * 4;

		for ( ; x < width - 2; x += 2)
		{
			COPYSAMPLE(outbyte, inbyte);
			outbyte += 8;
			inbyte += 8;
		}
	}

	for (y = 1; y < height - 1; y += 2)
	{
		byte sd[4] = {0}, se[4] = {0}, sh[4] = {0}, si[4] = {0};
		byte *line2, *line3;

		x = 1;

		line2 = in + ((y - 1) * width + (x - 1)) * 4;
		line3 = in + ((y + 1) * width + (x - 1)) * 4;

		COPYSAMPLE(sd, line2); line2 += 8;
		COPYSAMPLE(sh, line3); line3 += 8;

		outbyte = out + (y * width + x) * 4;

		for ( ; x < width - 1; x += 2)
		{
			COPYSAMPLE(se, line2); line2 += 8;
			COPYSAMPLE(si, line3); line3 += 8;

			for (i = 0; i < 4; i++)
			{	
				*outbyte++ = (sd[i] + si[i] + se[i] + sh[i]) >> 2;
			}

			outbyte += 4;

			COPYSAMPLE(sd, se);
			COPYSAMPLE(sh, si);
		}
	}

	// hack: copy out to in again
	for (y = 1; y < height - 1; y += 2)
	{
		x = 1;

		inbyte  = out + (y * width + x) * 4;
		outbyte = in  + (y * width + x) * 4;

		for ( ; x < width - 1; x += 2)
		{
			COPYSAMPLE(outbyte, inbyte);
			outbyte += 8;
			inbyte += 8;
		}
	}
	
	for (y = 1; y < height - 1; y++)
	{
		byte sd[4], sf[4], sg[4], si[4];
		byte *line2, *line3, *line4;

		x = y % 2 + 1;

		line2 = in + ((y - 1) * width + (x    )) * 4;
		line3 = in + ((y    ) * width + (x - 1)) * 4;
		line4 = in + ((y + 1) * width + (x    )) * 4;

		COPYSAMPLE(sf, line3); line3 += 8;

		outbyte = out + (y * width + x) * 4;

		for ( ; x < width - 1; x += 2)
		{
			COPYSAMPLE(sd, line2); line2 += 8;
			COPYSAMPLE(sg, line3); line3 += 8;
			COPYSAMPLE(si, line4); line4 += 8;

			for (i = 0; i < 4; i++)
			{
				*outbyte++ = (sf[i] + sg[i] + sd[i] + si[i]) >> 2;
			}

			outbyte += 4;

			COPYSAMPLE(sf, sg);
		}
	}
}


static void ExpandHalfTextureToGrid( byte *data, int width, int height)
{
	int x, y;

	for (y = height / 2; y > 0; y--)
	{
		byte *outbyte = data + ((y * 2 - 1) * (width)     - 2) * 4;
		byte *inbyte  = data + (y           * (width / 2) - 1) * 4;

		for (x = width / 2; x > 0; x--)
		{
			COPYSAMPLE(outbyte, inbyte);

			outbyte -= 8;
			inbyte -= 4;
		}
	}
}

static void FillInNormalizedZ(const byte *in, byte *out, int width, int height)
{
	int x, y;

	for (y = 0; y < height; y++)
	{
		const byte *inbyte  = in  + y * width * 4;
		byte       *outbyte = out + y * width * 4;

		for (x = 0; x < width; x++)
		{
			byte nx, ny, nz, h;
			float fnx, fny, fll, fnz;

			nx = *inbyte++;
			ny = *inbyte++;
			inbyte++;
			h  = *inbyte++;

			fnx = OffsetByteToFloat(nx);
			fny = OffsetByteToFloat(ny);
			fll = 1.0f - fnx * fnx - fny * fny;
			if (fll >= 0.0f)
				fnz = (float)sqrt(fll);
			else
				fnz = 0.0f;

			nz = FloatToOffsetByte(fnz);

			*outbyte++ = nx;
			*outbyte++ = ny;
			*outbyte++ = nz;
			*outbyte++ = h;
		}
	}
}


// size must be even
#define WORKBLOCK_SIZE     128
#define WORKBLOCK_BORDER   4
#define WORKBLOCK_REALSIZE (WORKBLOCK_SIZE + WORKBLOCK_BORDER * 2)

// assumes that data has already been expanded into a 2x2 grid
static void FCBIByBlock(byte *data, int width, int height, qboolean clampToEdge, qboolean normalized)
{
	byte workdata[WORKBLOCK_REALSIZE * WORKBLOCK_REALSIZE * 4];
	byte outdata[WORKBLOCK_REALSIZE * WORKBLOCK_REALSIZE * 4];
	byte *inbyte, *outbyte;
	int x, y;
	int srcx, srcy;

	ExpandHalfTextureToGrid(data, width, height);

	for (y = 0; y < height; y += WORKBLOCK_SIZE)
	{
		for (x = 0; x < width; x += WORKBLOCK_SIZE)
		{
			int x2, y2;
			int workwidth, workheight, fullworkwidth, fullworkheight;

			workwidth =  MIN(WORKBLOCK_SIZE, width  - x);
			workheight = MIN(WORKBLOCK_SIZE, height - y);

			fullworkwidth =  workwidth  + WORKBLOCK_BORDER * 2;
			fullworkheight = workheight + WORKBLOCK_BORDER * 2;

			//memset(workdata, 0, WORKBLOCK_REALSIZE * WORKBLOCK_REALSIZE * 4);

			// fill in work block
			for (y2 = 0; y2 < fullworkheight; y2 += 2)
			{
				srcy = y + y2 - WORKBLOCK_BORDER;

				if (clampToEdge)
				{
					srcy = CLAMP(srcy, 0, height - 2);
				}
				else
				{
					srcy = (srcy + height) % height;
				}

				outbyte = workdata + y2   * fullworkwidth * 4;
				inbyte  = data     + srcy * width         * 4;		

				for (x2 = 0; x2 < fullworkwidth; x2 += 2)
				{
					srcx = x + x2 - WORKBLOCK_BORDER;

					if (clampToEdge)
					{
						srcx = CLAMP(srcx, 0, width - 2);
					}
					else
					{
						srcx = (srcx + width) % width;
					}

					COPYSAMPLE(outbyte, inbyte + srcx * 4);
					outbyte += 8;
				}
			}

			// submit work block
			DoLinear(workdata, outdata, fullworkwidth, fullworkheight);

			if (!normalized) {
				switch (r_imageUpsampleType->i) {
				case 0:
					break;
				case 1:
					DoFCBIQuick(workdata, outdata, fullworkwidth, fullworkheight, 0);
					break;
				case 2:
				default:
					DoFCBI(workdata, outdata, fullworkwidth, fullworkheight, 0);
					break;
				};
			}
			else {
				switch (r_imageUpsampleType->i) {
				case 0:
					break;
				case 1:
					DoFCBIQuick(workdata, outdata, fullworkwidth, fullworkheight, 0);
					DoFCBIQuick(workdata, outdata, fullworkwidth, fullworkheight, 1);
					break;
				case 2:
				default:
					DoFCBI(workdata, outdata, fullworkwidth, fullworkheight, 0);
					DoFCBI(workdata, outdata, fullworkwidth, fullworkheight, 1);
					break;
				};
			}

			// copy back work block
			for (y2 = 0; y2 < workheight; y2++) {
				inbyte = outdata + ((y2 + WORKBLOCK_BORDER) * fullworkwidth + WORKBLOCK_BORDER) * 4;
				outbyte = data +   ((y + y2)                * width         + x)                * 4;
				for (x2 = 0; x2 < workwidth; x2++) {
					COPYSAMPLE(outbyte, inbyte);
					outbyte += 4;
					inbyte += 4;
				}
			}
		}
	}
}
#undef COPYSAMPLE

/*
================
R_LightScaleTexture

Scale up the pixel fs in a texture to increase the
lighting range
================
*/
static void R_LightScaleTexture (byte *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma ) {
		if ( !glConfig.deviceSupportsGamma ) {
			int i, c;
			byte	*p;

			p = in;

			c = inwidth*inheight;
			for (i=0 ; i<c ; i++, p+=4) {
				p[0] = s_gammatable[p[0]];
				p[1] = s_gammatable[p[1]];
				p[2] = s_gammatable[p[2]];
			}
		}
	}
	else {
		int i, c;
		byte	*p;

		p = in;

		c = inwidth*inheight;

		if ( glConfig.deviceSupportsGamma ) {
			for (i=0 ; i<c ; i++, p+=4)
			{
				p[0] = s_intensitytable[p[0]];
				p[1] = s_intensitytable[p[1]];
				p[2] = s_intensitytable[p[2]];
			}
		}
		else {
			for (i=0 ; i<c ; i++, p+=4) {
				p[0] = s_gammatable[s_intensitytable[p[0]]];
				p[1] = s_gammatable[s_intensitytable[p[1]]];
				p[2] = s_gammatable[s_intensitytable[p[2]]];
			}
		}
	}
}

/*
================
R_MipMapsRGB

Operates in place, quartering the size of the texture
Colors are gamma correct 
================
*/
static void R_MipMapsRGB( byte *in, int inWidth, int inHeight)
{
	int x, y, c, stride;
	const byte *in2;
	float total;
	static float downmipSrgbLookup[256];
	static int downmipSrgbLookupSet = 0;
	byte *out = in;

	if (!downmipSrgbLookupSet) {
		for (x = 0; x < 256; x++)
			downmipSrgbLookup[x] = powf(x / 255.0f, 2.2f) * 0.25f;
		downmipSrgbLookupSet = 1;
	}

	if (inWidth == 1 && inHeight == 1)
		return;

	if (inWidth == 1 || inHeight == 1) {
		for (x = (inWidth * inHeight) >> 1; x; x--) {
			for (c = 3; c; c--, in++) {
				total  = (downmipSrgbLookup[*(in)] + downmipSrgbLookup[*(in + 4)]) * 2.0f;

				*out++ = (byte)(powf(total, 1.0f / 2.2f) * 255.0f);
			}
			*out++ = (*(in) + *(in + 4)) >> 1; in += 5;
		}
		
		return;
	}

	stride = inWidth * 4;
	inWidth >>= 1; inHeight >>= 1;

	in2 = in + stride;
	for (y = inHeight; y; y--, in += stride, in2 += stride) {
		for (x = inWidth; x; x--) {
			for (c = 3; c; c--, in++, in2++) {
				total = downmipSrgbLookup[*(in)]  + downmipSrgbLookup[*(in + 4)]
				      + downmipSrgbLookup[*(in2)] + downmipSrgbLookup[*(in2 + 4)];

				*out++ = (byte)(powf(total, 1.0f / 2.2f) * 255.0f);
			}

			*out++ = (*(in) + *(in + 4) + *(in2) + *(in2 + 4)) >> 2; in += 5, in2 += 5;
		}
	}
}


static void R_MipMapNormalHeight (const byte *in, byte *out, int width, int height, qboolean swizzle)
{
	int		i, j;
	int		row;
	int sx = swizzle ? 3 : 0;
	int sa = swizzle ? 0 : 3;

	if ( width == 1 && height == 1 ) {
		return;
	}

	row = width * 4;
	width >>= 1;
	height >>= 1;
	
	for (i=0 ; i<height ; i++, in+=row) {
		for (j=0 ; j<width ; j++, out+=4, in+=8) {
			vec3_t v;

			v[0] =  OffsetByteToFloat(in[sx      ]);
			v[1] =  OffsetByteToFloat(in[       1]);
			v[2] =  OffsetByteToFloat(in[       2]);

			v[0] += OffsetByteToFloat(in[sx    +4]);
			v[1] += OffsetByteToFloat(in[       5]);
			v[2] += OffsetByteToFloat(in[       6]);

			v[0] += OffsetByteToFloat(in[sx+row  ]);
			v[1] += OffsetByteToFloat(in[   row+1]);
			v[2] += OffsetByteToFloat(in[   row+2]);

			v[0] += OffsetByteToFloat(in[sx+row+4]);
			v[1] += OffsetByteToFloat(in[   row+5]);
			v[2] += OffsetByteToFloat(in[   row+6]);

			VectorNormalizeFast(v);

			//v[0] *= 0.25f;
			//v[1] *= 0.25f;
			//v[2] = 1.0f - v[0] * v[0] - v[1] * v[1];
			//v[2] = sqrt(MAX(v[2], 0.0f));

			out[sx] = FloatToOffsetByte(v[0]);
			out[1 ] = FloatToOffsetByte(v[1]);
			out[2 ] = FloatToOffsetByte(v[2]);
			out[sa] = MAX(MAX(in[sa], in[sa+4]), MAX(in[sa+row], in[sa+row+4]));
		}
	}
}


/*
==================
R_BlendOverTexture

Apply a color blend over a set of pixels
==================
*/
static void R_BlendOverTexture( byte *data, uint32_t pixelCount, const byte blend[4] ) {
	uint32_t i;
	uint32_t inverseAlpha;
	uint32_t premult[3];

	inverseAlpha = 255 - blend[3];
	premult[0] = blend[0] * blend[3];
	premult[1] = blend[1] * blend[3];
	premult[2] = blend[2] * blend[3];

	for ( i = 0 ; i < pixelCount ; i++, data+=4 ) {
		data[0] = ( data[0] * inverseAlpha + premult[0] ) >> 9;
		data[1] = ( data[1] * inverseAlpha + premult[1] ) >> 9;
		data[2] = ( data[2] * inverseAlpha + premult[2] ) >> 9;
	}
}

static const byte	mipBlendColors[16][4] = {
	{0,0,0,0},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
	{255,0,0,128},
	{0,255,0,128},
	{0,0,255,128},
};

static void RawImage_SwizzleRA( byte *data, int width, int height )
{
	int i;
	byte *ptr = data, swap;

	for (i=0; i<width*height; i++, ptr+=4)
	{
		// swap red and alpha
		swap = ptr[0];
		ptr[0] = ptr[3];
		ptr[3] = swap;
	}
}

/*
===============
RawImage_ScaleToPower2

===============
*/
static qboolean RawImage_ScaleToPower2( byte **data, int *inout_width, int *inout_height, imgType_t type, imgFlags_t flags, byte **resampledBuffer)
{
	int width =         *inout_width;
	int height =        *inout_height;
	int scaled_width;
	int scaled_height;
	qboolean picmip = flags & IMGFLAG_PICMIP;
	qboolean mipmap = flags & IMGFLAG_MIPMAP;
	qboolean clampToEdge = flags & IMGFLAG_CLAMPTOEDGE;
	qboolean scaled;

	//
	// convert to exact power of 2 sizes
	//
	if (!mipmap) {
		scaled_width = width;
		scaled_height = height;
	}
	else {
		scaled_width = NextPowerOfTwo(width);
		scaled_height = NextPowerOfTwo(height);
	}

	if ( r_roundImagesDown->i && scaled_width > width )
		scaled_width >>= 1;
	if ( r_roundImagesDown->i && scaled_height > height )
		scaled_height >>= 1;

	if ( picmip && data && resampledBuffer && r_imageUpsample->i && 
	     scaled_width < r_imageUpsampleMaxSize->i && scaled_height < r_imageUpsampleMaxSize->i)
	{
		int finalwidth, finalheight;
		uint64_t startTime, endTime;

		startTime = ri.Milliseconds();

		finalwidth = scaled_width << r_imageUpsample->i;
		finalheight = scaled_height << r_imageUpsample->i;

		while ( finalwidth > r_imageUpsampleMaxSize->i
			|| finalheight > r_imageUpsampleMaxSize->i ) {
			finalwidth >>= 1;
			finalheight >>= 1;
		}

		while ( finalwidth > glConfig.maxTextureSize
			|| finalheight > glConfig.maxTextureSize ) {
			finalwidth >>= 1;
			finalheight >>= 1;
		}

		*resampledBuffer = ri.Hunk_AllocateTempMemory( finalwidth * finalheight * 4 );

		if (scaled_width != width || scaled_height != height)
			ResampleTexture (*data, width, height, *resampledBuffer, scaled_width, scaled_height);
		else
			memcpy(*resampledBuffer, *data, width * height * 4);

		if (type == IMGTYPE_COLORALPHA)
			RGBAtoYCoCgA(*resampledBuffer, *resampledBuffer, scaled_width, scaled_height);

		while (scaled_width < finalwidth || scaled_height < finalheight) {
			scaled_width <<= 1;
			scaled_height <<= 1;

			FCBIByBlock(*resampledBuffer, scaled_width, scaled_height, clampToEdge, (type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT));
		}

		if (type == IMGTYPE_COLORALPHA)
			YCoCgAtoRGBA(*resampledBuffer, *resampledBuffer, scaled_width, scaled_height);
		else if (type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT)
			FillInNormalizedZ(*resampledBuffer, *resampledBuffer, scaled_width, scaled_height);

		endTime = ri.Milliseconds();

		ri.Printf(PRINT_INFO, "upsampled %dx%d to %dx%d in %lums\n", width, height, scaled_width, scaled_height, endTime - startTime);

		*data = *resampledBuffer;
	}
	else if ( scaled_width != width || scaled_height != height ) {
		if (data && resampledBuffer) {
			*resampledBuffer = ri.Hunk_AllocateTempMemory( scaled_width * scaled_height * 4 );
			ResampleTexture (*data, width, height, *resampledBuffer, scaled_width, scaled_height);
			*data = *resampledBuffer;
		}
	}

	width  = scaled_width;
	height = scaled_height;

	//
	// perform optional picmip operation
	//
	if ( picmip ) {
		scaled_width >>= r_picmip->i;
		scaled_height >>= r_picmip->i;
	}

	//
	// clamp to the current upper OpenGL limit
	// scale both axis down equally so we don't have to
	// deal with a half mip resampling
	//
	while ( scaled_width > glConfig.maxTextureSize
		|| scaled_height > glConfig.maxTextureSize ) {
		scaled_width >>= 1;
		scaled_height >>= 1;
	}

	//
	// clamp to minimum size
	//
	scaled_width  = MAX(1, scaled_width);
	scaled_height = MAX(1, scaled_height);

	scaled = (width != scaled_width) || (height != scaled_height);

	//
	// rescale texture to new size using existing mipmap functions
	//
	if (data) {
		while (width > scaled_width || height > scaled_height) {
			if (type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT)
				R_MipMapNormalHeight(*data, *data, width, height, qfalse);
			else
				R_MipMapsRGB(*data, width, height);

			width  = MAX(1, width >> 1);
			height = MAX(1, height >> 1);
		}
	}

	*inout_width  = width;
	*inout_height = height;

	return scaled;
}

static qboolean RawImage_HasAlpha( const byte *scan, uint32_t numPixels )
{
	uint32_t i;

	if ( !scan ) {
		return qtrue;
	}
	
	for ( i = 0; i < numPixels; i++ ) {
		if ( scan[i*4 + 3] != 255 ) {
			return qtrue;
		}
	}
	return qfalse;
}

static GLenum RawImage_GetFormat( const byte *data, uint32_t numPixels, GLenum picFormat, qboolean lightMap, imgType_t type, imgFlags_t flags )
{
	uint32_t samples = 3;
	GLenum internalFormat = GL_RGB;
	qboolean forceNoCompression = ( flags & IMGFLAG_NO_COMPRESSION );
	qboolean normalmap = ( type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT );

	if ( picFormat != GL_RGBA8 ) {
		return picFormat;
	}

	if ( normalmap ) {
		if ( ( type == IMGTYPE_NORMALHEIGHT ) && RawImage_HasAlpha( data, numPixels ) ) {
			if ( !forceNoCompression && glContext.textureCompressionRef & TCR_BPTC ) {
				internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
			}
			else if ( !forceNoCompression && glContext.textureCompression == TC_S3TC_ARB ) {
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			else if ( r_textureBits->i == 16 ) {
				if ( r_hdr->i ) {
					internalFormat = GL_RGBA16F;
				} else {
					internalFormat = GL_RGBA4;
				}
			}
			else if ( r_textureBits->i == 32 ) {
				if ( r_hdr->i ) {
					internalFormat = GL_RGBA32F;
				} else {
					internalFormat = GL_RGBA8;
				}
			}
			else {
				switch ( r_textureDetail->i ) {
				case TexDetail_MSDOS:
				case TexDetail_IntegratedGPU:
					internalFormat = GL_RGBA4;
					break;
				case TexDetail_Normie:
					internalFormat = GL_RGBA8;
					break;
				case TexDetail_ExpensiveShitWeveGotHere: {
					if ( r_hdr->i ) {
						internalFormat = GL_RGBA16F;
					} else {
						internalFormat = GL_RGBA8;
					}
					break; }
				case TexDetail_GPUvsGod: {
					if ( r_hdr->i ) {
						internalFormat = GL_RGBA32F;
					} else {
						internalFormat = GL_RGBA8;
					}
					break; }
				};
			}
		}
		else {
			if ( !forceNoCompression && glContext.textureCompressionRef & TCR_RGTC ) {
				internalFormat = GL_COMPRESSED_RG_RGTC2;
			}
			else if ( !forceNoCompression && glContext.textureCompressionRef & TCR_BPTC ) {
				internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
			}
			else if ( !forceNoCompression && glContext.textureCompression == TC_S3TC_ARB ) {
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			else if ( r_textureBits->i == 16 ) {
				if ( r_hdr->i ) {
					internalFormat = GL_RGB16F;
				} else {
					internalFormat = GL_RGB5;
				}
			}
			else if ( r_textureBits->i == 32 ) {
				if ( r_hdr->i ) {
					internalFormat = GL_RGB32F;
				} else {
					internalFormat = GL_RGB8;
				}
			}
			else {
				switch ( r_textureDetail->i ) {
				case TexDetail_MSDOS:
				case TexDetail_IntegratedGPU:
					internalFormat = GL_RGB5;
					break;
				case TexDetail_Normie:
					internalFormat = GL_RGB8;
					break;
				case TexDetail_ExpensiveShitWeveGotHere: {
					if ( r_hdr->i ) {
						internalFormat = GL_RGB16F;
					} else {
						internalFormat = GL_RGB8;
					}
					break; }
				case TexDetail_GPUvsGod: {
					if ( r_hdr->i ) {
						internalFormat = GL_RGB32F;
					} else {
						internalFormat = GL_RGB8;
					}
					break; }
				};
			}
		}
	}
	else if (lightMap) {
		if (r_greyscale->i)
			internalFormat = GL_LUMINANCE;
		else
			internalFormat = GL_RGBA;
	}
	else {
		if (RawImage_HasAlpha(data, numPixels)) {
			samples = 4;
		}

		// select proper internal format
		if ( samples == 3 ) {
			if ( r_greyscale->i ) {
				if ( r_textureBits->i == 16 || r_textureBits->i == 32 ) {
					internalFormat = GL_LUMINANCE8;
				} else {
					internalFormat = GL_LUMINANCE;
				}
			}
			else {
				if ( !forceNoCompression && (glContext.textureCompressionRef & TCR_BPTC) ) {
					internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
				}
				else if ( !forceNoCompression && glContext.textureCompression == TC_S3TC_ARB ) {
					internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				}
				else if ( !forceNoCompression && glContext.textureCompression == TC_S3TC ) {
					internalFormat = GL_RGB4_S3TC;
				}
				else if ( r_textureBits->i == 16 ) {
					if ( r_hdr->i ) {
						internalFormat = GL_RGB16F;
					} else {
						internalFormat = GL_RGB5;
					}
				}
				else if ( r_textureBits->i == 32 ) {
					if ( r_hdr->i ) {
						internalFormat = GL_RGB32F;
					} else {
						internalFormat = GL_RGB8;
					}
				}
				else {
					switch ( r_textureDetail->i ) {
					case TexDetail_MSDOS:
					case TexDetail_IntegratedGPU:
						internalFormat = GL_RGB4;
						break;
					case TexDetail_Normie:
						internalFormat = GL_RGB8;
						break;
					case TexDetail_ExpensiveShitWeveGotHere: {
						if ( r_hdr->i ) {
							internalFormat = GL_RGB16F;
						} else {
							internalFormat = GL_RGB8;
						}
						break; }
					case TexDetail_GPUvsGod: {
						if ( r_hdr->i ) {
							internalFormat = GL_RGB32F;
						} else {
							internalFormat = GL_RGB8;
						}
						break; }
					};
				}
			}
		}
		else if ( samples == 4 ) {
			if ( r_greyscale->i ) {
				if ( r_textureBits->i == 16 || r_textureBits->i == 32 ) {
					internalFormat = GL_LUMINANCE8_ALPHA8;
				} else {
					internalFormat = GL_LUMINANCE_ALPHA;
				}
			}
			else {
				if ( !forceNoCompression && (glContext.textureCompressionRef & TCR_BPTC) ) {
					internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
				}
				else if ( !forceNoCompression && glContext.textureCompression == TC_S3TC_ARB ) {
					internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				}
				else if ( r_textureBits->i == 16 ) {
					if ( r_hdr->i ) {
						internalFormat = GL_RGBA16F;
					} else {
						internalFormat = GL_RGBA4;
					}
				}
				else if ( r_textureBits->i == 32 ) {
					if ( r_hdr->i ) {
						internalFormat = GL_RGBA32F;
					} else {
						internalFormat = GL_RGBA8;
					}
				}
				else {
					switch ( r_textureDetail->i ) {
					case TexDetail_MSDOS:
					case TexDetail_IntegratedGPU:
						internalFormat = GL_RGBA4;
						break;
					case TexDetail_Normie:
						internalFormat = GL_RGBA8;
						break;
					case TexDetail_ExpensiveShitWeveGotHere: {
						if ( r_hdr->i ) {
							internalFormat = GL_RGBA16F;
						} else {
							internalFormat = GL_RGBA8;
						}
						break; }
					case TexDetail_GPUvsGod: {
						if ( r_hdr->i ) {
							internalFormat = GL_RGBA32F;
						} else {
							internalFormat = GL_RGBA8;
						}
						break; }
					};
				}
			}
		}
	}

	return internalFormat;
}

static void CompressMonoBlock( byte outdata[8], const byte indata[16] )
{
	uint32_t hi, lo, diff, bias, outbyte, shift, i;
	byte *p = outdata;

	hi = lo = indata[0];
	for (i = 1; i < 16; i++) {
		hi = MAX(indata[i], hi);
		lo = MIN(indata[i], lo);
	}

	*p++ = hi;
	*p++ = lo;

	diff = hi - lo;

	if (diff == 0) {
		outbyte = (hi == 255) ? 255 : 0;

		for (i = 0; i < 6; i++)
			*p++ = outbyte;

		return;
	}

	bias = diff / 2 - lo * 7;
	outbyte = shift = 0;
	for (i = 0; i < 16; i++) {
		const byte fixIndex[8] = { 1, 7, 6, 5, 4, 3, 2, 0 };
		byte index = fixIndex[(indata[i] * 7 + bias) / diff];

		outbyte |= index << shift;
		shift += 3;
		if (shift >= 8)
		{
			*p++ = outbyte & 0xff;
			shift -= 8;
			outbyte >>= 8;
		}
	}
}

static void RawImage_UploadToRgtc2Texture( uint32_t mipLevel, int x, int y, int width, int height, byte *data, qboolean subImage )
{
	int wBlocks, hBlocks, iy, ix;
	uint32_t size;
	byte *compressedData, *p;

	wBlocks = (width + 3) / 4;
	hBlocks = (height + 3) / 4;
	size = wBlocks * hBlocks * 16;

	p = compressedData = ri.Hunk_AllocateTempMemory(size);
	for (iy = 0; iy < height; iy += 4) {
		int oh = MIN(4, height - iy);

		for (ix = 0; ix < width; ix += 4) {
			byte workingData[16];
			int component;

			int ow = MIN(4, width - ix);

			for (component = 0; component < 2; component++) {
				int ox, oy;

				for (oy = 0; oy < oh; oy++)
					for (ox = 0; ox < ow; ox++)
						workingData[oy * 4 + ox] = data[((iy + oy) * width + ix + ox) * 4 + component];

				// dupe data to fill
				for (oy = 0; oy < 4; oy++)
					for (ox = (oy < oh) ? ow : 0; ox < 4; ox++)
						workingData[oy * 4 + ox] = workingData[(oy % oh) * 4 + ox % ow];

				CompressMonoBlock(p, workingData);
				p += 8;
			}
		}
	}

	// FIXME: Won't work for x/y that aren't multiples of 4.
	if (subImage) {
		GL_LogComment("glCompressedTexSubImage2D(GL_TEXTURE_2D, %i, %i, %i, %i, %i, GL_COMPRESSED_RG_RGTC2, %u, %p)",
			mipLevel, x, y, width, height, size, compressedData);
		nglCompressedTexSubImage2D(GL_TEXTURE_2D, mipLevel, x, y, width, height, GL_COMPRESSED_RG_RGTC2, size, compressedData);
	}
	else {
		GL_LogComment("glCompressedTexImage2D(GL_TEXTURE_2D, %i, GL_COMPRESSED_RG_RGTC2, %i, %i, 0, %u, %p)", mipLevel, width, height, size, compressedData);
		nglCompressedTexImage2D(GL_TEXTURE_2D, mipLevel, GL_COMPRESSED_RG_RGTC2, width, height, 0, size, compressedData);
	}

	ri.Hunk_FreeTempMemory(compressedData);
}

static uint64_t CalculateTextureSize( int width, int height, GLenum picFormat) 
{
	uint64_t numBlocks = ((width + 3) / 4) * ((height + 3) / 4);
	uint64_t numPixels = width * height;

	switch (picFormat) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
		return numBlocks * 8;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
	case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
		return numBlocks * 16;
	case GL_RGB8:
		return numPixels * 3;
	case GL_RGBA8:
	case GL_SRGB8_ALPHA8_EXT:
		return numPixels * 4;
	case GL_RGBA16:
		return numPixels * 8;
	default:
		ri.Printf(PRINT_INFO, "Unsupported texture format 0x%08x\n", picFormat);
		return 0;
	};

	return 0;
}


static GLenum PixelDataFormatFromInternalFormat( GLenum internalFormat )
{
	switch ( internalFormat ) {
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16_ARB:
	case GL_DEPTH_COMPONENT24_ARB:
	case GL_DEPTH_COMPONENT32_ARB:
		return GL_DEPTH_COMPONENT;
	default:
		return GL_RGBA;
	};
}

static int PixelDataFormatIsValidCompressed( GLenum format )
{
	switch (format) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_SIGNED_RED_RGTC1:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_SIGNED_RG_RGTC2:
	case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
	case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
	case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
	case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
	case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
		return qtrue;
	default:
		break;
	};
	return qfalse;
};

static void RawImage_UploadTexture( GLuint texture, byte *data, int x, int y, int width, int height, GLenum target, GLenum picFormat,
	int numMips, GLenum internalFormat, imgType_t type, imgFlags_t flags, qboolean subtexture )
{
	GLenum dataFormat, dataType;
	qboolean rgtc = internalFormat == GL_COMPRESSED_RG_RGTC2;
	qboolean rgba8 = picFormat == GL_RGBA8 || picFormat == GL_SRGB8_ALPHA8_EXT;
	qboolean rgba = rgba8 || picFormat == GL_RGBA16;
	qboolean mipmap = !!(flags & IMGFLAG_MIPMAP);
	uint64_t size, miplevel;
	qboolean lastMip = qfalse;

	dataFormat = PixelDataFormatFromInternalFormat(internalFormat);
	dataType = picFormat == GL_RGBA16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;

	if ( internalFormat == GL_RGBA16F || internalFormat == GL_RGBA32F ) {
		dataType = GL_FLOAT;
	}

	miplevel = 0;
	do {
		lastMip = (width == 1 && height == 1) || !mipmap;
		size = CalculateTextureSize(width, height, picFormat);

		if (!rgba && !(flags & IMGFLAG_NO_COMPRESSION) && PixelDataFormatIsValidCompressed(picFormat)) {
			GL_LogComment("glCompressedTexSubImage2D(GL_TEXTURE_2D, %lu, %i, %i, %i, %i, 0x%04x, %lu, %p)", miplevel, x, y, width, height, picFormat, size, data);
			nglCompressedTexSubImage2D(GL_TEXTURE_2D, miplevel, x, y, width, height, picFormat, size, data);
		}
		else {
			if (rgba8 && miplevel != 0 && r_colorMipLevels->i)
				R_BlendOverTexture((byte *)data, width * height, mipBlendColors[miplevel]);

			if (rgba8 && rgtc)
				RawImage_UploadToRgtc2Texture(miplevel, x, y, width, height, data, subtexture);
			else if (subtexture) {
				GL_LogComment("glTexSubImage2D(GL_TEXTURE_2D, %lu, %i, %i, %i, %i, 0x%04x, %i, %p)", miplevel, x, y, width, height, dataFormat, dataType, data);
				nglTexSubImage2D(target, miplevel, x, y, width, height, dataFormat, dataType, data);
			}
			else {
				GL_LogComment( "glTexImage2D(GL_TEXTURE_2D, %lu, %i, %i, %i, %i, 0x%04x, %i, %p)", miplevel, internalFormat, width, height, 0, dataFormat, dataType, data );
				nglTexImage2D( target, miplevel, internalFormat, width, height, 0, dataFormat, dataType, data );
			}
		}

		if (!lastMip && numMips < 2) {
			if (glContext.ARB_framebuffer_object) {
				GL_LogComment("Generating mipmap for texture object %i", texture);
				nglBindTexture( target, texture );
				nglGenerateMipmap( target );
				break;
			}
			else if (rgba8) {
				if (type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT)
					R_MipMapNormalHeight(data, data, width, height, glContext.swizzleNormalmap);
				else
					R_MipMapsRGB(data, width, height);
			}
		}

		x >>= 1;
		y >>= 1;
		width = MAX(1, width >> 1);
		height = MAX(1, height >> 1);
		miplevel++;

		if (numMips > 1)
		{
			data += size;
			numMips--;
		}
	} while (!lastMip);
}

/*
===============
Upload32

===============
*/
static void Upload32( byte *data, int x, int y, int width, int height, GLenum picFormat, uint32_t numMips, texture_t *image, qboolean scaled )
{
	int	i, c;
	byte		*scan;

	imgType_t type = image->type;
	imgFlags_t flags = image->flags;
	GLenum internalFormat = image->internalFormat;
	qboolean rgba8 = picFormat == GL_RGBA8 || picFormat == GL_SRGB8_ALPHA8_EXT;
	qboolean mipmap = !!(flags & IMGFLAG_MIPMAP) && (rgba8 || numMips > 1);
	qboolean cubemap = !!(flags & IMGFLAG_CUBEMAP);

	// These operations cannot be performed on non-rgba8 images.
	if ( rgba8 && !cubemap && !( flags & IMGFLAG_FBO ) ) {
		c = width*height;
		scan = data;

		if ( type == IMGTYPE_COLORALPHA ) {
			if( r_greyscale->i ) {
				for ( i = 0; i < c; i++ ) {
					byte luma = LUMA(scan[i*4], scan[i*4 + 1], scan[i*4 + 2]);
					scan[i*4] = luma;
					scan[i*4 + 1] = luma;
					scan[i*4 + 2] = luma;
				}
			}
			else if( r_greyscale->f ) {
				for ( i = 0; i < c; i++ ) {
					float luma = LUMA(scan[i*4], scan[i*4 + 1], scan[i*4 + 2]);
					scan[i*4] = LERP(scan[i*4], luma, r_greyscale->f);
					scan[i*4 + 1] = LERP(scan[i*4 + 1], luma, r_greyscale->f);
					scan[i*4 + 2] = LERP(scan[i*4 + 2], luma, r_greyscale->f);
				}
			}

			// This corresponds to what the OpenGL1 renderer does.
			if ( !( flags & IMGFLAG_NOLIGHTSCALE ) /* && ( scaled || mipmap ) */ ) {
				R_LightScaleTexture( data, width, height, !mipmap );
			}
		}

		if ( glContext.swizzleNormalmap && ( type == IMGTYPE_NORMAL || type == IMGTYPE_NORMALHEIGHT ) ) {
			RawImage_SwizzleRA( data, width, height );
		}
	}

	if (cubemap) {
		for (i = 0; i < 6; i++) {
			int w2 = width, h2 = height;
			RawImage_UploadTexture(image->id, data, x, y, width, height, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, picFormat, numMips, internalFormat, type, flags, qfalse);
			for (c = numMips; c; c--) {
				data += CalculateTextureSize(w2, h2, picFormat);
				w2 = MAX(1, w2 >> 1);
				h2 = MAX(1, h2 >> 1);
			}
		}
	}
	else {
		RawImage_UploadTexture( image->id, data, x, y, width, height, GL_TEXTURE_2D, picFormat, numMips, internalFormat, type, flags, qfalse );
	}

}

/*
================
R_CreateImage2

This is the only way any texture_t are created
================
*/
static texture_t *R_CreateImage2( const char *name, byte *pic, int width, int height, GLenum picFormat, int numMips, imgType_t type, imgFlags_t flags, int internalFormat )
{
	texture_t *image;
	uint64_t namelen, hash;
	GLenum glWrapClampMode;
	GLenum dataType;
	GLenum dataFormat;
	qboolean rgba8 = picFormat == GL_RGBA8 || picFormat == GL_SRGB8_ALPHA8;
	qboolean isLightmap = qfalse, scaled = qfalse;
	byte *resampledBuffer = NULL;
	int lastMip, mipWidth, mipHeight, miplevel;
	qboolean mipmap = !!( flags & IMGFLAG_MIPMAP );
	uint32_t estSize = 0;

	namelen = strlen( name );
	if ( namelen >= MAX_NPATH ) {
		ri.Error( ERR_DROP, "R_CreateImage: name \"%s\" too long", name );
	}
	if ( !strncmp( name, "*lightmap", 9 ) ) {
		isLightmap = qtrue;
	}

	if ( rg.numTextures == MAX_RENDER_TEXTURES ) {
		ri.Error( ERR_DROP, "R_CreateImage: MAX_RENDER_TEXTURES hit" );
	}
	
	image = rg.textures[ rg.numTextures ] = (texture_t *)ri.Hunk_Alloc( sizeof( *image ) + namelen, h_low );
	nglGenTextures(1, &image->id);
	rg.numTextures++;

	image->flags = flags;
	image->type = type;

	image->imgName = (char *)( image + 1 );
	strcpy( image->imgName, name );

	image->width = width;
	image->height = height;
	if ( flags & IMGFLAG_CLAMPTOBORDER ) {
		glWrapClampMode = GL_CLAMP_TO_BORDER;
	} else if ( flags & IMGFLAG_CLAMPTOEDGE ) {
		glWrapClampMode = GL_CLAMP_TO_EDGE;
	} else {
		glWrapClampMode = GL_REPEAT;
	}

	if ( !internalFormat ) {
		internalFormat = RawImage_GetFormat( pic, width * height, picFormat, isLightmap, image->type, image->flags );
	}
	image->internalFormat = internalFormat;

	// possibly scale image before uploading
	/* this breaks the textures
	if ( rgba8 ) {
		ri.Printf( PRINT_DEVELOPER, "Scaling '%s' to power of two...\n", name );
		scaled = RawImage_ScaleToPower2( &pic, &width, &height, type, flags, &resampledBuffer );
	}
	*/

	image->uploadWidth = width;
	image->uploadHeight = height;

	// Allocate texture storage so we don't have to worry about it later
	dataFormat = PixelDataFormatFromInternalFormat( internalFormat );

	GL_BindTexture( TB_COLORMAP, image );

/*
	mipWidth = width;
	mipHeight = height;
	miplevel = 0;
	do
	{
		lastMip = !mipmap || (mipWidth == 1 && mipHeight == 1);
		if (cubemap)
		{
			int i;

			for (i = 0; i < 6; i++)
				nglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, miplevel, internalFormat, mipWidth, mipHeight, 0, dataFormat, GL_UNSIGNED_BYTE, NULL );
		}
		else
		{
			nglTexImage2D( GL_TEXTURE_2D, miplevel, internalFormat, mipWidth, mipHeight, 0, dataFormat, GL_UNSIGNED_BYTE, NULL );
		}

		mipWidth  = MAX(1, mipWidth >> 1);
		mipHeight = MAX(1, mipHeight >> 1);
		miplevel++;
	}
	while (!lastMip);
*/

	// Upload data.
	if ( pic || ( flags & IMGFLAG_FBO ) ) {
		GL_LogComment( "-- (%s) -- Upload32( 0x%04x, %i, %i, 0x%04x, 0x%04x, %p )", name, image->internalFormat, width, height, dataFormat, dataType, pic );
		Upload32( pic, 0, 0, width, height, picFormat, numMips, image, scaled );
	}
	if ( resampledBuffer != NULL ) {
		ri.Hunk_FreeTempMemory( resampledBuffer );
	}

	// set all necessary parameters
	nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapClampMode );
	nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapClampMode );

	// set anisotropy
	if ( r_arb_texture_filter_anisotropic->i ) {
		nglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY,
			(GLint)Com_Clamp( 1, glContext.maxAnisotropy, r_arb_texture_max_anisotropy->i ) );
	}

	switch ( internalFormat ) {
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

	GL_BindTexture( TB_COLORMAP, NULL );

	GL_CheckErrors();
	
	estSize += image->uploadWidth * image->uploadHeight;
	switch ( image->internalFormat ) {
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		// 64 bits per 16 pixels, so 4 bits per pixel
		estSize /= 2;
		break;
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		// 128 bits per 16 pixels, so 1 byte per pixel
		break;
	case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
		// 128 bits per 16 pixels, so 1 byte per pixel
		break;
	case GL_COMPRESSED_RG_RGTC2:
		// 128 bits per 16 pixels, so 1 byte per pixel
		break;
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		// 64 bits per 16 pixels, so 4 bits per pixel
		estSize /= 2;
		break;
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		// 64 bits per 16 pixels, so 4 bits per pixel
		estSize /= 2;
		break;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		// 128 bits per 16 pixels, so 1 byte per pixel
		break;
	case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
		// 128 bits per 16 pixels, so 1 byte per pixel
		break;
	case GL_RGB4_S3TC:
		// same as DXT1?
		estSize /= 2;
		break;
	case GL_RGBA16F_ARB:
		// 8 bytes per pixel
		estSize *= 16;
		break;
	case GL_RGBA16:
		// 8 bytes per pixel
		estSize *= 8;
		break;
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGBA8:
	case GL_RGBA12:
	case GL_RGBA:
	case GL_RGB12:
		// 4 bytes per pixel
		estSize *= 4;
		break;
	case GL_RGBA32F_ARB:
	case GL_RGB32F_ARB:
		// 16 bytes per pixel?
		estSize *= 16;
		break;
	case GL_LUMINANCE8:
	case GL_LUMINANCE:
		// 1 byte per pixel?
		break;
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB2_EXT:
	case GL_RGB:
		// 3 bytes per pixel?
		estSize *= 3;
		break;
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE_ALPHA:
		// 2 bytes per pixel?
		estSize *= 2;
		break;
	case GL_SRGB_EXT:
	case GL_SRGB8_EXT:
		// 3 bytes per pixel?
		estSize *= 3;
		break;
	case GL_SRGB_ALPHA_EXT:
	case GL_SRGB8_ALPHA8_EXT:
		// 4 bytes per pixel?
		estSize *= 4;
		break;
	case GL_SLUMINANCE_EXT:
	case GL_SLUMINANCE8_EXT:
		// 1 byte per pixel?
		break;
	case GL_SLUMINANCE_ALPHA_EXT:
	case GL_SLUMINANCE8_ALPHA8_EXT:
		// 2 byte per pixel?
		estSize *= 2;
		break;
	case GL_DEPTH_COMPONENT16:
		// 2 bytes per pixel
		estSize *= 2;
		break;
	case GL_DEPTH_COMPONENT24:
		// 3 bytes per pixel
		estSize *= 3;
		break;
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT32:
		// 4 bytes per pixel
		estSize *= 4;
		break;
	default:
		ri.Printf( PRINT_WARNING, "unknown texture format 0x%04i\n", image->internalFormat );
	};
	glState.memstats.estTextureMemUsed += estSize;
	glState.memstats.numTextures++;
	
	if ( rg.world && rg.worldMapLoaded ) {
		rg.world->levelTextures++;
	}

	hash = generateHashValue(name);

	// link it in
	image->next = hashTable[hash];
	hashTable[hash] = image;
	
	return image;
}

/*
================
R_CreateImage

Wrapper for R_CreateImage2()
================
*/
texture_t *R_CreateImage( const char *name, byte *pic, int width, int height, imgType_t type, imgFlags_t flags, int internalFormat )
{
	return R_CreateImage2( name, pic, width, height, GL_RGBA8, 0, type, flags, internalFormat );
}

//===================================================================

// Prototype for dds loader function which isn't common to both renderers
void R_LoadDDS(const char *filename, byte **pic, uint32_t *width, uint32_t *height, GLenum *picFormat, uint32_t *numMips);

static void LoadImageFile( const char *name, byte **pic, int *width, int *height, int *channels )
{
	uint64_t size;
	byte *out;
	union {
		void *v;
		byte *b;
	} buffer;
	fileHandle_t f;

	//
	// load the file
	//
	size = ri.FS_LoadFile( name, &buffer.v );
	if ( !buffer.b || size == 0 ) {
		return;
	}

	out = stbi_load_from_memory( buffer.b, size, width, height, channels, STBI_rgb );
	if ( !out ) {
		ri.FS_FreeFile( buffer.b );
		ri.Printf( PRINT_DEVELOPER, "LoadImageFile: stbi_load_from_memory(%s) failed, failure reason: %s\n", name, stbi_failure_reason() );
		return;
	}

	ri.FS_FreeFile( buffer.v );

	*pic = out;
}

typedef struct
{
	const char *ext;
	void (*ImageLoader)( const char *, unsigned char **, int *, int *, int * );
} imageExtToLoaderMap_t;

static int stbi_eof( void *userData ) {
	fileHandle_t fd = *(fileHandle_t *)userData;
	return ri.FS_FileTell( fd ) >= ri.FS_FileLength( fd );
}

static int stbi_read( void *userData, char *buffer, int size ) {
	return ri.FS_Read( buffer, size, *(fileHandle_t *)userData );
}

static void stbi_skip( void *userData, int skip ) {
	if ( skip < 0 ) {
		ri.FS_FileSeek( *(fileHandle_t *)userData, ri.FS_FileTell( *(fileHandle_t *)userData ) - ( skip * -1 ), FS_SEEK_SET );
	} else {
		ri.FS_FileSeek( *(fileHandle_t *)userData, skip, FS_SEEK_CUR );
	}
}

static void R_LoadPNG2( const char *filename, unsigned char **pic, int *width, int *height, int *channels ) {
	union {
		void *v;
		char *b;
	} f;
	uint64_t nLength;

	nLength = ri.FS_LoadFile( filename, &f.v );
	if ( !nLength || !f.v ) {
		return;
	}

	*pic = stbi_load_from_memory( f.b, (int)nLength, width, height, channels, 3 );
	if ( !*pic ) {
		ri.Printf( PRINT_WARNING, "R_LoadPNG: stbi_load_from_memory(%s) failed, failure reason: %s\n", filename, stbi_failure_reason() );
		return;
	}

	ri.FS_FreeFile( f.v );
}

// Note that the ordering indicates the order of preference used
// when there are multiple images of different formats available
static const imageExtToLoaderMap_t imageLoaders[] = {
	{ "png",  R_LoadPNG2 },
	{ "tga",  R_LoadTGA },
	{ "jpg",  R_LoadJPG },
	{ "jpeg", R_LoadJPG },
	{ "pcx",  R_LoadPCX },
	{ "bmp",  R_LoadBMP },
	{ "webp", R_LoadWebp }
};

static const int numImageLoaders = arraylen( imageLoaders );

/*
=================
R_LoadImage

Loads any of the supported image types into a canonical
32 bit format.
=================
*/
static void R_LoadImage( const char *name, byte **pic, int *width, int *height, GLenum *picFormat, uint32_t *numMips )
{
	qboolean orgNameFailed = qfalse;
	char localName[ MAX_NPATH ];
	const char *ext;
	const char *altName;
	int orgLoader = -1;
	int i;
	int channels = 3;

	*pic = NULL;
	*width = 0;
	*height = 0;
	*picFormat = GL_RGBA8;
	*numMips = 0;

	N_strncpyz( localName, name, sizeof( localName ) );

	ext = COM_GetExtension( localName );

	// If compressed textures are enabled, try loading a DDS first, it'll load fastest
	if ( r_arb_texture_compression->i ) {
		char ddsName[MAX_NPATH];

		COM_StripExtension( name, ddsName, MAX_NPATH );
		N_strcat( ddsName, MAX_NPATH, "dds" );

		R_LoadDDS( ddsName, pic, width, height, picFormat, numMips );

		// If loaded, we're done.
		if ( *pic ) {
			return;
		}
	}

	if ( *ext ) {
		// Look for the correct loader and use it
		for ( i = 0; i < numImageLoaders; i++ ) {
			if ( !N_stricmp( ext, imageLoaders[ i ].ext ) ) {
				// Load
				imageLoaders[ i ].ImageLoader( localName, pic, width, height, &channels );
				break;
			}
		}

		// A loader was found
		if ( i < numImageLoaders ) {
			if ( *pic == NULL ) {
				// Loader failed, most likely because the file isn't there;
				// try again without the extension
				orgNameFailed = qtrue;
				orgLoader = i;
				COM_StripExtension( name, localName, MAX_NPATH );
			}
			else {
				*picFormat = channels == 4 ? GL_RGBA8 : GL_RGB8;
				// Something loaded
				return;
			}
		}
	}

	// Try and find a suitable match using all
	// the image formats supported
	for ( i = 0; i < numImageLoaders; i++ ) {
		if ( i == orgLoader ) {
			continue;
		}

		altName = va( "%s.%s", localName, imageLoaders[ i ].ext );

		// Load
		imageLoaders[ i ].ImageLoader( altName, pic, width, height, &channels );

		if ( *pic ) {
			if ( orgNameFailed ) {
				ri.Printf( PRINT_DEVELOPER, "WARNING: %s not present, using %s instead\n",
						name, altName );
			}

			break;
		}
	}
	*picFormat = channels == 4 ? GL_RGBA8 : GL_RGB8;
}

/*
===============
R_FindImageFile

Finds or loads the given image.
Returns NULL if it fails, not a default image.
==============
*/
texture_t *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags )
{
	texture_t *image;
	uint32_t width, height;
	byte *pic;
	GLenum picFormat;
	int picNumMips;
	uint64_t hash;
	imgFlags_t checkFlagsTrue, checkFlagsFalse;

	if ( !name ) {
		return NULL;
	}

	hash = generateHashValue( name );

	//
	// see if the image is already loaded
	//
	for ( image = hashTable[hash]; image; image = image->next ) {
		if ( !N_strcmp( name, image->imgName ) ) {
			// the white image can be used with any set of parms, but other mismatches are errors
			if ( N_strcmp( name, "*white" ) ) {
				if ( image->flags != flags ) {
					ri.Printf( PRINT_DEVELOPER, "WARNING: reused image %s with mixed flags (%i vs %i)\n", name, image->flags, flags );
				}
			}
			return image;
		}
	}

	//
	// load the pic from disk
	//
	R_LoadImage( name, &pic, &width, &height, &picFormat, &picNumMips );
	if ( pic == NULL ) {
		return NULL;
	}

	checkFlagsTrue = IMGFLAG_GENNORMALMAP;
	checkFlagsFalse = IMGFLAG_CUBEMAP;
	if ( r_normalMapping->i && ( picFormat == GL_RGBA8 ) && ( type == IMGTYPE_COLORALPHA ) &&
		( ( flags & checkFlagsTrue ) == checkFlagsTrue ) && !( flags & checkFlagsFalse ) )
	{
		char normalName[MAX_NPATH];
		texture_t *normalImage;
		uint32_t normalWidth, normalHeight;
		imgFlags_t normalFlags;

		normalFlags = ( flags & ~IMGFLAG_GENNORMALMAP ) | IMGFLAG_NOLIGHTSCALE;

		COM_StripExtension( name, normalName, MAX_NPATH );
		N_strcat( normalName, MAX_NPATH, "_n" );

		// find normalmap in case it's there
		normalImage = R_FindImageFile( normalName, IMGTYPE_NORMAL, normalFlags );

		// if not, generate it
		if ( normalImage == NULL && r_genNormalMaps->i ) {
			byte *normalPic;
			uint32_t x, y;

			normalWidth = width;
			normalHeight = height;
			normalPic = ri.Hunk_AllocateTempMemory( width * height * 4 );
			RGBAtoNormal( pic, normalPic, width, height, flags & IMGFLAG_CLAMPTOEDGE );

#if 1
			// Brighten up the original image to work with the normal map
			RGBAtoYCoCgA( pic, pic, width, height );
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

			R_CreateImage( normalName, normalPic, normalWidth, normalHeight, IMGTYPE_NORMAL, normalFlags, 0 );
			ri.Hunk_FreeTempMemory( normalPic );	
		}
	}

	// force mipmaps off if image is compressed but doesn't have enough mips
	if ( ( flags & IMGFLAG_MIPMAP ) && picFormat != GL_RGBA8 && picFormat != GL_SRGB8_ALPHA8_EXT ) {
		int wh = MAX( width, height );
		int neededMips = 0;
		while ( wh ) {
			neededMips++;
			wh >>= 1;
		}
		if ( neededMips > picNumMips ) {
			flags &= ~IMGFLAG_MIPMAP;
		}
	}

	image = R_CreateImage( name, pic, width, height, type, flags, 0 );
	ri.Free( pic );
	return image;
}

#define DEFAULT_SIZE 16
static void R_CreateDefaultTexture( void )
{
	uint32_t x;
	byte data[DEFAULT_SIZE][DEFAULT_SIZE][4];

	// the default image will be a box, to allow you to see the mapping coordinates
	memset( data, 32, sizeof( data ) );
	for ( x = 0; x < DEFAULT_SIZE; x++ ) {
		data[0][x][0] =
		data[0][x][1] =
		data[0][x][2] =
		data[0][x][3] = 255;

		data[x][0][0] =
		data[x][0][1] =
		data[x][0][2] =
		data[x][0][3] = 255;

		data[DEFAULT_SIZE-1][x][0] =
		data[DEFAULT_SIZE-1][x][1] =
		data[DEFAULT_SIZE-1][x][2] =
		data[DEFAULT_SIZE-1][x][3] = 255;

		data[x][DEFAULT_SIZE-1][0] =
		data[x][DEFAULT_SIZE-1][1] =
		data[x][DEFAULT_SIZE-1][2] =
		data[x][DEFAULT_SIZE-1][3] = 255;
	}
	rg.defaultImage = R_CreateImage( "*default", (byte *)data, DEFAULT_SIZE, DEFAULT_SIZE, IMGTYPE_COLORALPHA, IMGFLAG_MIPMAP, 0 );
}

static void R_CreateBuiltinTextures( void )
{
	uint32_t x, y;
	byte data[DEFAULT_SIZE][DEFAULT_SIZE][4];

	R_CreateDefaultTexture();

	// we use a solid white image instead of disabling texturing
	memset( data, 255, sizeof( data ) );
	rg.whiteImage = R_CreateImage( "*white", (byte *)data, 8, 8, IMGTYPE_COLORALPHA, IMGFLAG_NONE, 0 );

	// with overbright bits active, we need an image which is some fraction of full color,
	// for default lightmaps, etc
	for ( x = 0; x < DEFAULT_SIZE; x++ ) {
		for ( y = 0; y < DEFAULT_SIZE; y++ ) {
			data[y][x][0] = 
			data[y][x][1] = 
			data[y][x][2] = rg.identityLightByte;
			data[y][x][3] = 255;			
		}
	}

	rg.identityLightImage = R_CreateImage( "*identityLight", (byte *)data, 8, 8, IMGTYPE_COLORALPHA, IMGFLAG_NONE, 0 );

	for ( x = 0 ; x < arraylen( rg.scratchImage ) ; x++ ) {
		// scratchimage is usually used for cinematic drawing
//		rg.scratchImage[x] = R_CreateImage( "*scratch", (byte *)data, DEFAULT_SIZE, DEFAULT_SIZE, IMGTYPE_COLORALPHA, IMGFLAG_PICMIP | IMGFLAG_CLAMPTOEDGE, 0 );
	}

//	R_CreateDlightImage();
//	R_CreateFogImage();

	if ( glContext.ARB_framebuffer_object ) {
		int width, height, hdrFormat, rgbFormat;

		width = glConfig.vidWidth;
		height = glConfig.vidHeight;
		switch ( r_multisampleType->i ) {
		case AntiAlias_2xSSAA:
			width *= 2;
			height *= 2;
			break;
		case AntiAlias_4xSSAA:
			width *= 4;
			height *= 4;
			break;
		};

		hdrFormat = GL_RGBA8;
		if ( r_hdr->i && glContext.ARB_texture_float ) {
			hdrFormat = GL_RGBA16F_ARB;
		}

		rgbFormat = GL_RGBA8;

		rg.renderImage = R_CreateImage( "_render", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, hdrFormat );

		if ( r_shadowBlur->i ) {
			rg.screenScratchImage = R_CreateImage( "screenScratch", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, rgbFormat );
		}

		if ( r_shadowBlur->i || r_ssao->i ) {
			rg.hdrDepthImage = R_CreateImage( "*hdrDepth", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_R32F );
		}

		if ( r_drawSunRays->i ) {
			rg.sunRaysImage = R_CreateImage("*sunRays", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, rgbFormat);
		}
		if ( r_multisampleType->i == AntiAlias_SMAA ) {
			float *p;

			p = (float *)ri.Hunk_AllocateTempMemory( sizeof( *p ) * width * height );
			for ( y = 0; y < height; y++ ) {
				for ( x = 0; x < width; x++ ) {
					p[ y * width + x ] = 1.0f;
				}
			}
			rg.smaaBlendImage = R_CreateImage( "*smaaBlendImage", (byte *)p, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, GL_RGBA16F );
			ri.Hunk_FreeTempMemory( p );

			rg.smaaEdgesImage = R_CreateImage( "*smaaEdgesImage", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_RGBA8 );
			rg.smaaWeightsImage = R_CreateImage( "*smaaWeightsImage", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_RGBA8 );
			rg.smaaAreaImage = R_FindImageFile( "textures/effects/SearchTex.jpg", IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE );
			rg.smaaSearchImage = R_FindImageFile( "textures/effects/AreaTexDX10.jpg", IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE );
		}

		if ( r_bloom->i && r_hdr->i ) {
//			rg.bloomImage = R_CreateImage( "*bloom", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, GL_RGBA16F );
		}

		if ( r_multisampleType->i == AntiAlias_2xSSAA || r_multisampleType->i == AntiAlias_4xSSAA ) {
			int max = gl_filter_max;
			gl_filter_max = GL_NEAREST;

			rg.renderDepthImage  = R_CreateImage( "*renderdepth",  NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_DEPTH_COMPONENT24 );

			gl_filter_max = max;
		} else {
			rg.renderDepthImage  = R_CreateImage( "*renderdepth",  NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_DEPTH_COMPONENT24 );
		}
		rg.textureDepthImage = R_CreateImage( "*texturedepth", NULL, PSHADOW_MAP_SIZE, PSHADOW_MAP_SIZE, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_DEPTH_COMPONENT24 );

		{
			void *p;

			data[0][0][0] = 0;
			data[0][0][1] = 0.45f * 255;
			data[0][0][2] = 255;
			data[0][0][3] = 255;
			p = data;

			rg.calcLevelsImage =   R_CreateImage( "*calcLevels",    p, 1, 1, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, hdrFormat );
			rg.targetLevelsImage = R_CreateImage( "*targetLevels",  p, 1, 1, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, hdrFormat );
			rg.fixedLevelsImage =  R_CreateImage( "*fixedLevels",   p, 1, 1, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, hdrFormat );
		}

		for ( x = 0; x < 2; x++ ) {
			rg.textureScratchImage[x] = R_CreateImage( va( "*textureScratch%d", x), NULL, 256, 256, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_RGBA8 );
		}
		for ( x = 0; x < 2; x++ ) {
			rg.quarterImage[x] = R_CreateImage( va( "*quarter%d", x ), NULL, width / 2, height / 2, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_RGBA8 );
		}

		if ( r_ssao->i ) {
			rg.screenSsaoImage = R_CreateImage( "*screenSsao", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE | IMGFLAG_FBO, GL_RGBA8 );
		}

		/* GL error on this
		if ( r_sunlightMode->i ) {
			for ( x = 0; x < 4; x++ ) {
				rg.sunShadowDepthImage[x] = R_CreateImage( va( "*sunshadowdepth%i", x ), NULL, r_shadowMapSize->i, r_shadowMapSize->i, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, GL_DEPTH_COMPONENT24 );
				GL_BindTexture( TB_DIFFUSEMAP, rg.sunShadowDepthImage[x] );
				nglTexParameterf( rg.sunShadowDepthImage[x]->id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
				nglTexParameterf( rg.sunShadowDepthImage[x]->id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
			}

			rg.screenShadowImage = R_CreateImage( "*screenShadow", NULL, width, height, IMGTYPE_COLORALPHA, IMGFLAG_NO_COMPRESSION | IMGFLAG_CLAMPTOEDGE, GL_RGBA8 );
		}
		*/
	}
}

/*
===============
R_SetColorMappings
===============
*/
void R_SetColorMappings( void )
{
	uint64_t i, j;
	float g;
	int inf;

	// setup the overbright lighting
	rg.overbrightBits = r_overBrightBits->i;

	// allow 2 overbright bits
	if ( rg.overbrightBits > 2 ) {
		rg.overbrightBits = 2;
	} else if ( rg.overbrightBits < 0 ) {
		rg.overbrightBits = 0;
	}
	rg.identityLight = 1.0f / ( 1 << 2 );
	rg.identityLightByte = 255 * rg.identityLight;


	if ( r_intensity->f <= 1 ) {
		ri.Cvar_Set( "r_intensity", "1" );
	}

	g = r_gammaAmount->f;

	for ( i = 0; i < 256; i++ ) {
		if ( g == 1 ) {
			inf = i;
		} else {
			inf = 255 * powf( i / 255.0f, 1.0f / g ) + 0.5f;
		}

		if ( inf < 0 ) {
			inf = 0;
		}
		if ( inf > 255 ) {
			inf = 255;
		}
		s_gammatable[i] = inf;
	}

	for ( i = 0; i < 256; i++ ) {
		j = i * r_intensity->f;
		if ( j > 255 ) {
			j = 255;
		}
		s_intensitytable[i] = j;
	}

	if ( glConfig.deviceSupportsGamma ) {
		ri.GLimp_SetGamma( s_gammatable, s_gammatable, s_gammatable );
	}
}

void R_InitTextures( void )
{
	memset( hashTable, 0, sizeof( hashTable ) );

	R_UpdateTextures();

	// build brightness translation tables
	R_SetColorMappings();

	// create default texture and white texture
	R_CreateDefaultTexture();

	// create the builtin textures for the framebuffers
	R_CreateBuiltinTextures();
}

void R_DeleteTextures( void )
{
	uint64_t i;

	for ( i = 0; i < rg.numTextures; i++ ) {
		if ( !rg.textures[i] ) {
			continue;
		}
		nglDeleteTextures( 1, &rg.textures[i]->id );
	}
	memset( rg.textures, 0, sizeof( rg.textures ) );

	GL_BindNullTextures();
}

void R_UnloadLevelTextures( void )
{
	uint64_t i, j;

	ri.Printf( PRINT_INFO, "R_UnloadLevelTextures(): Releasing %u textures...\n", rg.world->levelTextures );

	for ( i = 0; i < FILE_HASH_SIZE; i++ ) {
		for ( j = 0; j < rg.world->levelTextures; j++ ) {
			if ( hashTable[i] == rg.textures[ j + rg.world->firstLevelTexture ] && hashTable[i] ) {
				hashTable[i]->next = NULL;
				hashTable[i] = NULL;
			}
		}
	}
	for ( j = 0; j < rg.world->levelTextures; j++ ) {
		nglDeleteTextures( 1, &rg.textures[ j + rg.world->firstLevelTexture ]->id );
		rg.textures[ j + rg.world->firstLevelTexture ] = NULL;
	}
//	memset( rg.textures + rg.world->firstLevelTexture, 0, sizeof( *rg.textures ) * rg.world->levelTextures );
	rg.numTextures = rg.world->firstLevelTexture;
}