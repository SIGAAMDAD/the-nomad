#ifndef __RGL_VAO__
#define __RGL_VAO__

#pragma once

#include "../engine/n_allocator.h"
#include <EASTL/fixed_vector.h>

typedef enum
{
    BUF_GL_MAPPED,
    BUF_GL_BUFFER,
    BUF_GL_CLIENT,
} bufferMemType_t;

// not meant to be used by anything other than the vbo backend

typedef enum {
    BUFFER_STATIC,      // data is constant throughout buffer lifetime
    BUFFER_DYNAMIC,     // expected to be updated once in a while, but not every frame
    BUFFER_FRAME,       // expected to be update on a per-frame basis
    BUFFER_STREAM,      // use GL_STREAM_DRAW -- only really used by the imgui backend
} bufferType_t;

typedef struct {
    void *m_pData;
    uint64_t m_nSize;
    uintptr_t m_nOffset;
    bufferMemType_t m_Usage;
    uint32_t m_Id;
    uint32_t m_Target;
    uint32_t m_GLusage;
} buffer_t;

typedef struct vertexAttrib_s {
    uint32_t index;
    uint32_t count;
    uint32_t type;
    uint32_t enabled;
    uint32_t normalized;
    uintptr_t stride;
    uintptr_t offset;

    GDR_INLINE vertexAttrib_s( uint32_t index, uint32_t count, uint32_t type, uint32_t enabled, uint32_t normalized, uintptr_t stride, uintptr_t offset )
    {
        this->index = index;
        this->count = count;
        this->type = type;
        this->enabled = enabled;
        this->normalized = normalized;
        this->stride = stride;
        this->offset = offset;
    }
} vertexAttrib_t;

class CVertexCache
{
public:
    CVertexCache( void ) = default;
    ~CVertexCache() = default;

    void Init( const char *name, uint64_t verticesSize, void *vertices, uint64_t indicesSize, void *indices, bufferType_t type );
    void Shutdown( void );

    void Bind( void ) const;
    void Unbind( void ) const;

    const buffer_t *GetIndexObject( void ) const;
    const buffer_t *GetVertexObject( void ) const;

    uint32_t GetIndexBuffer( void ) const;
    uint32_t GetVertexBuffer( void ) const;
    void SetAttributes( const eastl::vector<vertexAttrib_t, CHunkTempAllocator>& attribs );
    const vertexAttrib_t *GetAttributes( void ) const;

    void SetVertexPointers( void ) const;
private:
    char m_szName[MAX_GDR_PATH];

    uint32_t m_VaoId;
    bufferType_t m_Type;

    buffer_t m_hVertex;
    buffer_t m_hIndex;

    vertexAttrib_t m_Attribs[ATTRIB_INDEX_COUNT];
};

GDR_EXPORT CVertexCache *R_AllocateBuffer( const char *name, uint64_t verticesSize, void *vertices, uint64_t indicesSize, void *indices, bufferType_t type );
GDR_EXPORT void VBO_Bind( CVertexCache *vbo );
GDR_EXPORT void VBO_BindNull( void );
GDR_EXPORT void R_InitGPUBuffers( void );
GDR_EXPORT void R_ShutdownGPUBuffers( void );

// for batch drawing
GDR_EXPORT void RB_SetBatchBuffer( CVertexCache *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize );
GDR_EXPORT void RB_FlushBatchBuffer( void );
GDR_EXPORT void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices );

GDR_EXPORT GDR_INLINE void CVertexCache::Bind( void ) const
{
    nglBindVertexArray( m_VaoId );
    if (glContext.intelGraphics) {
        nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_hIndex.m_Id );
    }
}

GDR_EXPORT GDR_INLINE void CVertexCache::Unbind( void ) const
{
    nglBindVertexArray( 0 );
    if (glContext.intelGraphics) {
        nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
}

GDR_EXPORT GDR_INLINE void CVertexCache::SetAttributes( const eastl::vector<vertexAttrib_t, CHunkTempAllocator>& attribs )
{
    memcpy( m_Attribs, attribs.data(), sizeof(m_Attribs) );
    SetVertexPointers();
}

GDR_EXPORT GDR_INLINE const buffer_t *CVertexCache::GetIndexObject( void ) const
{
    return &m_hIndex;
}

GDR_EXPORT GDR_INLINE const buffer_t *CVertexCache::GetVertexObject( void ) const
{
    return &m_hVertex;
}

GDR_EXPORT GDR_INLINE const vertexAttrib_t *CVertexCache::GetAttributes( void ) const
{
    return m_Attribs;
}

GDR_EXPORT GDR_INLINE uint32_t CVertexCache::GetIndexBuffer( void ) const
{
    return m_hIndex.m_Id;
}

GDR_EXPORT GDR_INLINE uint32_t CVertexCache::GetVertexBuffer( void ) const
{
    return m_hVertex.m_Id;
}

#endif