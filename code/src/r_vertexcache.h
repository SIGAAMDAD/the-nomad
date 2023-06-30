#ifndef _R_VERTEXCACHE_
#define _R_VERTEXCACHE_

#pragma once

#define RENDER_MAX_QUADS 0x80000
#define RENDER_MAX_VERTICES (RENDER_MAX_QUADS*4)
#define RENDER_MAX_INDICES (RENDER_MAX_VERTICES*6)

typedef struct vertex_s
{
	glm::vec3 pos;
	glm::vec2 texcoords;
	glm::vec4 color;
	glm::vec2 background;
	float type;
	
	GDR_INLINE vertex_s(const glm::vec3& _pos, const glm::vec2& _texcoords, const glm::vec4& _color)
		: pos(_pos), texcoords(+texcoords), color(_color) { }
	GDR_INLINE vertex_s()
		: pos(0.0f), texcoords(0.0f), color(0.0f) { }
	GDR_INLINE vertex_s(const vertex_s &v)
		: pos(v.pos), texcoords(v.texcoords), color(v.color) { }
	GDR_INLINE vertex_s(vertex_s &&v)
		: pos(v.pos), texcoords(v.texcoords), color(v.color) { }
	GDR_INLINE ~vertex_s() = default;
} vertex_t;

typedef struct
{
	uint32_t index;
	uint32_t count;
	uint32_t type;
	uint32_t offset;
} vertexAttrib_t;

class VertexCache
{
public:
	VertexCache();
	VertexCache(const VertexCache &) = delete;
	VertexCache(VertexCache &&) = delete;
	~VertexCache();
	
	VertexCache& operator=(const VertexCache &) = delete;
	VertexCache& operator=(VertexCache &&) = delete;
	
	void Init(void);
	void Shutdown(void);
	
	void ClearVertexData(void);
	void ClearIndexData(void);
	void ClearData(void);
	
	void SwapVertexData(uint64_t offset, const vertex_t *_vertices, uint64_t _numVertices);
	void SwapIndexData(uint64_t offset, const void *_indices, uint64_t _numIndices);
	
	void Bind(void) const;
	void Unbind(void) const;
	void BindVertexArray(void) const;
	void BindVertexBuffer(void) const;
	void BindIndexBuffer(void) const;
	void UnbindVertexArray(void) const;
	void UnbindVertexBuffer(void) const;
	void UnbindIndexBuffer(void) const;
	
	/* c-styled interface (method replacements) */
	// opengl stuff
	friend VertexCache* RGL_InitCache(const vertex_t *vertices, uint64_t numVertices,
		const void *indices, uint64_t numIndices, uint32_t indexType);
	friend void RGL_ShutdownCache(VertexCache *cache);
	friend void RGL_SwapVertexData(const vertex_t *vertices, uint64_t numVertices, VertexCache *cache);
	friend void RGL_SwapIndexData(const void *indices, uint64_t numIndices, VertexCache *cache);
	friend void RGL_BindCache(const VertexCache *cache);
	friend void RGL_BindVertexArray(const VertexCache *cache);
	friend void RGL_BindVertexBuffer(const VertexCache *cache);
	friend void RGL_BindIndexBuffer(const VertexCache *cache);
	friend void RGL_UnbindCache(void);
	friend void RGL_UnbindVertexArray(void);
	friend void RGL_UnbindVertexBuffer(void);
	friend void RGL_UnbindIndexBuffer(void);
    friend void RGL_DrawCache(VertexCache *cache);
    friend void RGL_ResetVertexData(VertexCache *cache);
    friend void RGL_ResetIndexData(VertexCache *cache);
private:
	typedef union {
		uint32_t glObj;
		VkBuffer vkObj[2]; // staging buffer and gpu buffer
	} cacheObject_t;
	
	vertex_t vertices[RENDER_MAX_VERTICES];
	uint32_t indices[RENDER_MAX_INDICES];
	
	uint64_t numVertices;
	uint64_t numIndices;
	
	void *attribs;
	
	VkVertexInputBindingDescription attribDescription;
	
	// if the buffer has been changed to warrant a gpu buffer swap
	qboolean vboChanged;
	qboolean iboChanged;
	
	uint32_t vaoId;
	cacheObject_t vbo;
	cacheObject_t ibo;
	
	uint32_t indexType;
	uint32_t indexSize;
	
	void InitVertexBuffer(void);
	void InitIndexBuffer(void);
};

typedef VertexCache vertexCache_t;

VertexCache* RGL_InitCache(const vertex_t *vertices, uint64_t numVertices,
	const void *indices, uint64_t numIndices, uint32_t indexType);
void RGL_ShutdownCache(VertexCache *cache);
void RGL_SwapVertexData(const vertex_t *vertices, uint64_t numVertices, VertexCache *cache);
void RGL_SwapIndexData(const void *indices, uint64_t numIndices, VertexCache *cache);
void RGL_BindCache(const VertexCache *cache);
void RGL_BindVertexArray(const VertexCache *cache);
void RGL_BindVertexBuffer(const VertexCache *cache);
void RGL_BindIndexBuffer(const VertexCache *cache);
void RGL_UnbindCache(void);
void RGL_UnbindVertexArray(void);
void RGL_UnbindVertexBuffer(void);
void RGL_UnbindIndexBuffer(void);
void RGL_DrawCache(VertexCache *cache);
void RGL_ResetVertexData(VertexCache *cache);
void RGL_ResetIndexData(VertexCache *cache);

#endif