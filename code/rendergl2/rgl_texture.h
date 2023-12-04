#ifndef __RGL_TEXTURE__
#define __RGL_TEXTURE__

#pragma once

typedef uint32_t imgFlags_t;
typedef uint32_t imgType_t;

enum
{
	IMGTYPE_COLORALPHA, // for color, lightmap, diffuse, and specular
	IMGTYPE_NORMAL,
	IMGTYPE_NORMALHEIGHT,
	IMGTYPE_DELUXE, // normals are swizzled, deluxe are not
};

enum
{
	IMGFLAG_NONE           = 0x0000,
	IMGFLAG_PICMIP         = 0x0001,
	IMGFLAG_NO_COMPRESSION = 0x0002,
	IMGFLAG_NOLIGHTSCALE   = 0x0004,
	IMGFLAG_CLAMPTOEDGE    = 0x0008,
	IMGFLAG_GENNORMALMAP   = 0x0010,
	IMGFLAG_LIGHTMAP       = 0x0020,
	IMGFLAG_NOSCALE        = 0x0040,
	IMGFLAG_CLAMPTOBORDER  = 0x0080,
    IMGFLAG_NOWRAP         = 0x0100
};

class CTexture
{
public:
    CTexture( void ) = default;
    ~CTexture() = default;

    static CTexture* CreateImage( const char *name, byte *pic, uint32_t width, uint32_t height, imgType_t type,
                                            imgFlags_t flags, int32_t internalFormat, GLenum picFormat );
    void Shutdown( void );

    void Bind( uint32_t slot = 0 ) const;
    void Unbind( void ) const;

    const char *GetName( void ) const;
    CTexture *GetNext( void );
    uint32_t GetWidth( void ) const;
    uint32_t GetHeight( void ) const;
    uint32_t GetId( void ) const;
    imgFlags_t GetFlags( void ) const;
private:
    char *m_pImageName;             // image path, including extension
    CTexture *m_pNext;              // for hash search
    CTexture *m_pList;              // for listing

    uint32_t m_nWidth, m_nHeight;   // source image
    int32_t m_InternalFormat;
    uint32_t m_Id;                  // GL texture binding

    imgType_t m_Type;
    imgFlags_t m_Flags;
};

GDR_EXPORT GDR_INLINE const char *CTexture::GetName( void ) const
{
    return m_pImageName;
}

GDR_EXPORT GDR_INLINE CTexture *CTexture::GetNext( void )
{
    return m_pNext;
}

GDR_EXPORT GDR_INLINE uint32_t CTexture::GetWidth( void ) const
{
    return m_nWidth;
}

GDR_EXPORT GDR_INLINE uint32_t CTexture::GetHeight( void ) const
{
    return m_nHeight;
}

GDR_EXPORT GDR_INLINE uint32_t CTexture::GetId( void ) const
{
    return m_Id;
}

GDR_EXPORT GDR_INLINE imgFlags_t CTexture::GetFlags( void ) const
{
    return m_Flags;
}

GDR_EXPORT void R_ImageList_f( void );
GDR_EXPORT void R_DeleteTextures( void );
GDR_EXPORT CTexture *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags );
//GDR_EXPORT void R_GammaCorrect( byte *buffer, uint64_t bufSize );
GDR_EXPORT void R_UpdateTextures( void );
GDR_EXPORT void R_InitTextures( void );

extern int32_t gl_filter_min, gl_filter_max;

#endif