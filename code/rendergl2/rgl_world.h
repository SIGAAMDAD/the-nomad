#ifndef __RGL_WORLD__
#define __RGL_WORLD__

#pragma once

#include "../engine/n_allocator.h"

class CRenderWorld
{
public:
    CRenderWorld( void ) = default;
    ~CRenderWorld() = default;

    void LoadLights( const lump_t *lights );
    void LoadTiles( const lump_t *tiles );
//    void LoadCheckpoints( const lump_t *c );
//    void LoadSpawns( const lump_t *s );
    void LoadTileset( const lump_t *sprites, const tile2d_header_t *theader );
    void GenerateDrawData( void );

    void SetName( const char *pFilename );
    uint32_t GetWidth( void ) const;
    uint32_t GetHeight( void ) const;
    void SetSize( uint32_t nWidth, uint32_t nHeight );

    const char *GetBaseName( void ) const;
    const char *GetName( void ) const;

    drawVert_t *GetVertices( void );
    glIndex_t *GetIndices( void );

    CVertexCache *GetDrawBuffer( void );

    CShader *m_pShader;
private:
    char m_szBaseName[MAX_GDR_PATH];
    char m_szName[MAX_GDR_PATH];

    uint32_t m_nWidth;
    uint32_t m_nHeight;

//    eastl::vector<maplight_t, CHunkAllocator> m_pLights;
    maplight_t *m_pLights;
    uint32_t m_nLights;

//    eastl::vector<maptile_t, CHunkAllocator> m_pTiles;
    maptile_t *m_pTiles;
    uint32_t m_nTiles;

    // uneeded
    /*
//    eastl::vector<mapcheckpoint_t, CHunkAllocator> m_pCheckpoints;
    mapcheckpoint_t *m_pCheckpoints;
    uint32_t m_nCheckpoints;

//    eastl::vector<mapspawn_t, CHunkAllocator> m_pSpawns;
    mapspawn_t *m_pSpawns;
    uint32_t m_nSpawns;
    */

    uint64_t m_nIndices;
    uint64_t m_nVertices;

//    eastl::vector<tile2d_sprite_t, CHunkAllocator> m_pSprites;
    tile2d_sprite_t *m_pSprites;
    uint32_t m_nTilesetSprites;

    // frame based draw data
    drawVert_t *m_pVertices;
    glIndex_t *m_pIndices;
    nhandle_t m_Tileset;

    CVertexCache *m_pDrawBuffer;
};

GDR_EXPORT GDR_INLINE void CRenderWorld::SetName( const char *pFilename )
{
    N_strncpyz( m_szName, pFilename, sizeof(m_szName) );
    N_strncpyz( m_szBaseName, COM_SkipPath( const_cast<char *>(pFilename) ), sizeof(m_szBaseName) );

    COM_StripExtension( m_szBaseName, m_szBaseName, sizeof(m_szBaseName) );
}

GDR_EXPORT GDR_INLINE void CRenderWorld::SetSize( uint32_t nWidth, uint32_t nHeight )
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nTiles = nWidth * nHeight;
}

GDR_EXPORT GDR_INLINE CVertexCache *CRenderWorld::GetDrawBuffer( void )
{
    return m_pDrawBuffer;
}

GDR_EXPORT GDR_INLINE uint32_t CRenderWorld::GetWidth( void ) const
{
    return m_nWidth;
}

GDR_EXPORT GDR_INLINE uint32_t CRenderWorld::GetHeight( void ) const
{
    return m_nHeight;
}

GDR_EXPORT GDR_INLINE drawVert_t *CRenderWorld::GetVertices( void )
{
    return m_pVertices;
}

GDR_EXPORT GDR_INLINE glIndex_t *CRenderWorld::GetIndices( void )
{
    return m_pIndices;
}

GDR_EXPORT GDR_INLINE const char *CRenderWorld::GetBaseName( void ) const
{
    return m_szBaseName;
}

GDR_EXPORT GDR_INLINE const char *CRenderWorld::GetName( void ) const
{
    return m_szName;
}



#endif