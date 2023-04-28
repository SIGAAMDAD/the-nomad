//----------------------------------------------------------
//
// Copyright (C) GDR Games 2022-2023
//
// This source code is available for distribution and/or
// modification under the terms of either the Apache License
// v2.0 as published by the Apache Software Foundation, or
// the GNU General Public License v2.0 as published by the
// Free Software Foundation.
//
// This source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY. If you are using this code for personal,
// non-commercial/monetary gain, you may use either of the
// licenses permitted, otherwise, you must use the GNU GPL v2.0.
//
// DESCRIPTION: src/g_zone.h
//  header for the zone allocation daemon
//----------------------------------------------------------
#ifndef _Z_ZONE_
#define _Z_ZONE_

#pragma once

enum {
	TAG_FREE = 0,
	TAG_STATIC = 1,
	TAG_LEVEL = 2,
	TAG_SFX = 3,
	TAG_MUSIC = 4,

	TAG_PURGELEVEL = 100,
	TAG_SCOPE = 101,
	TAG_LOAD = 102,
	TAG_CACHE = 103,

	NUMTAGS = 104
};

#ifdef _NOMAD_DEBUG
#define LOG_CALL(func) LOG_DEBUG("%s called from %s:%s:%iu",func,_FILE__,__func__,__LINE__)
#else
#define LOG_CALL(func)
#endif

extern "C" void *Z_Malloc(uint32_t size, uint8_t tag, void *user);
extern "C" void *Z_Realloc(void *ptr, uint32_t nsize, void *user, uint8_t tag);
extern "C" void *Z_Calloc(void *user, uint32_t elemsize, uint32_t nelem, uint8_t tag);
extern "C" void Z_CheckHeap();
extern "C" void Z_FileDumpHeap();
extern "C" void Z_DumpHeap();
extern "C" void Z_FreeTags(uint8_t lowtag, uint8_t hightag);
extern "C" void Z_ClearZone();
extern "C" uint32_t Z_ZoneSize();
extern "C" void Z_ChangeUser(void *ptr, void *user);
extern "C" void Z_ChangeTag2(void *ptr, uint8_t tag, const char* file, uint32_t line);
extern "C" void Z_CleanCache();
extern "C" void Z_ChangeTag(void *ptr, uint8_t tag);
extern "C" void Z_ScanForBlock(void *start, void *end);
extern "C" void Z_Init();
extern "C" void Z_Free(void *ptr);
extern "C" void Z_Print(bool all);

#if 0
#define Z_Malloc(size, tag, user)            (Z_Malloc)   (size,tag,user,       __FILE__,FUNC_SIG,__LINE__)
#define Z_Calloc(user, elemsize, nelem, tag) (Z_Calloc)   (user,elemsize,nelem, __FILE__,FUNC_SIG,__LINE__)
#define Z_Realloc(ptr, nsize, user)          (Z_Realloc)  (ptr,nsize,user,      __FILE__,FUNC_SIG,__LINE__)
#endif

#ifdef _WIN32
#define PROT_READ     0x1
#define PROT_WRITE    0x2
/* This flag is only available in WinXP+ */
#ifdef FILE_MAP_EXECUTE
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

#ifdef __USE_FILE_OFFSET64
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif
#endif

#ifdef _WIN32
extern "C" FILE* fmemopen(void *buf, size_t size, const char *mode);
extern "C" FILE* open_memstream(char **buf, size_t* size);
extern "C" void *mmap(void *start, size_t size, int32_t prot, int32_t flags, int32_t fd, off_t offset);
extern "C" void munmap(void *addr, size_t size);
#endif

inline void *N_malloc(size_t size)
{
	size += sizeof(uint64_t);
	void *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	*(uint64_t *)ptr = size;
	return (void *)((byte *)ptr + sizeof(uint64_t));
}
inline void *N_calloc(size_t elemsize, size_t nelem)
{
	return memset((N_malloc)(elemsize * nelem), 0, nelem * elemsize);
}
inline void N_Free(void *ptr)
{
	void *addr = (void *)((byte *)ptr - sizeof(uint64_t));
	munmap(addr, *(uint64_t *)addr);
}

template<typename T>
struct zone_deleter {
    constexpr void operator()(T* arg) const { Z_Free(arg); }
};

template<typename T>
inline std::unique_ptr<T, zone_deleter<T>> make_scoped_block(uint8_t tag = TAG_SCOPE) {
	return std::unique_ptr<T, zone_deleter<T>>(static_cast<T*>(Z_Malloc(sizeof(T), tag, NULL)));
}

template<typename T>
class zone_ptr
{
private:
	T *ptr;
public:
	zone_ptr(T *_ptr) noexcept
		: ptr(_ptr) { }
	template<typename... Args>
	zone_ptr(T *_ptr, Args&&... args) noexcept
		: ptr(_ptr)
	{
		new (ptr) T(std::forward<Args>(args)...);
	}
	zone_ptr(const zone_ptr &) noexcept = default;
	zone_ptr(zone_ptr &&) noexcept = default;

	constexpr zone_ptr(std::nullptr_t) noexcept : zone_ptr(){}
	constexpr zone_ptr() noexcept = default;

	~zone_ptr()
	{
		ptr->~T();
		Z_Free(ptr);
	}

	inline zone_ptr& operator=(const zone_ptr &) noexcept = default;
	inline T* operator->(void) noexcept { return ptr; }
};

template<class T>
struct nomad_allocator
{
	nomad_allocator() noexcept { }

	typedef T value_type;
	template<class U>
	constexpr nomad_allocator(const nomad_allocator<U>&) noexcept { }

	[[nodiscard]] inline T* allocate(std::size_t n) const {
		T* p;
		if ((p = static_cast<T*>(malloc(n))) != NULL)
			return p;
		
		throw std::bad_alloc();
	}
	[[nodiscard]] inline T* allocate(std::size_t& n, std::size_t& alignment, std::size_t& offset) const {
		T* p;
#if EASTL_ALIGNED_MALLOC_AVAILABLE
		if ((offset % alignment) == 0) {
			if((p = memalign(alignment, n)) != NULL) // memalign is more consistently available than posix_memalign.
				return p;
			else
				throw std::bad_alloc();
		}
#else
		if ((alignment <= EASTL_SYSTEM_ALLOCATOR_MIN_ALIGNMENT) && ((offset % alignment) == 0)) {
			if ((p = static_cast<T*>(malloc(n))) != NULL)
				return p;
			else
				throw std::bad_alloc();
		}
#endif
		return NULL;
	}
	void deallocate(void *p, std::size_t n) const noexcept {
		free(p);
	}
};

template<class T>
struct zone_allocator
{
	zone_allocator() noexcept { }

	typedef T value_type;
    template<class U>
    constexpr zone_allocator(const zone_allocator<U>&) noexcept { }

    [[nodiscard]] inline T* allocate(std::size_t n) const {
        T* p = NULL;
		if ((p = static_cast<T*>(Z_Realloc(p, n, &p, TAG_STATIC))) != NULL)
            return p;

        throw std::bad_alloc();
    }
	[[nodiscard]] inline T* allocate(std::size_t& n, std::size_t& alignment, std::size_t& offset) const {
		T* p = NULL;
		if ((p = static_cast<T*>(Z_Realloc(p, n, &p, TAG_STATIC))) != NULL)
			return p;
		
		throw std::bad_alloc();
	}
	void deallocate(void* p, std::size_t n) const noexcept {
		Z_Free(p);
	}
//	void deallocate(T* p, std::size_t n) noexcept {
//	    Z_Free(p);
//	}
};


template<typename T>
class linked_list
{
public:
	class linked_list_node
	{
	public:
		T val;
		linked_list_node *next;
		linked_list_node *prev;
	public:
		linked_list_node() = default;
		linked_list_node(const linked_list_node &) = delete;
		linked_list_node(linked_list_node &&) = default;
		~linked_list_node() = default;
		inline linked_list_node& operator++(int) noexcept
		{
			next->next->prev = this;
			next->prev = prev;
			next->next = this;
			prev->next = next;
			return *this;
		}
		inline linked_list_node& operator++() noexcept
		{
			next->next->prev = this;
			next->prev = prev;
			next->next = this;
			prev->next = next;
			return *this;
		}
	};
	
	typedef linked_list_node* node;
	typedef linked_list_node list_node;
	typedef linked_list_node* iterator;
	typedef const linked_list_node* const_iterator;
	typedef std::size_t size_type;
	typedef T value_type;
private:
	linked_list<T>::node ptr_list = NULL;
	std::size_t _size = 0;
	
	linked_list<T>::node alloc_node(void) {
		linked_list<T>::node ptr = (linked_list<T>::node)Z_Malloc(
			sizeof(linked_list<T>::list_node), TAG_STATIC, &ptr);
		if (ptr == NULL)
			N_Error("linked_list::alloc_node: memory allocation failed");
		
		return ptr;
	}
	void dealloc_node(linked_list<T>::node ptr) noexcept {
		if (ptr == NULL)
			return;
		Z_Free(ptr);
	}
public:
	linked_list() noexcept = default;
	~linked_list() noexcept
	{
		if (ptr_list == NULL || begin() == NULL)
			return;

		for (linked_list<T>::iterator it = begin(); it != end(); it = it->next)
			dealloc_node(it);
	}
	linked_list(linked_list &&) = default;
	linked_list(const linked_list &) = default;
	
	// memory management
	inline void clear(void) noexcept
	{
		if (ptr_list == NULL)
			return;
		
		for (linked_list<T>::iterator it = begin(); it != end(); it = it->next)
			dealloc_node(it);
	}
	inline void free_node(linked_list<T>::node ptr) noexcept
	{
		if (ptr == NULL)
			return;
		
		ptr->prev->next = ptr->next;
		ptr->next->prev = ptr->prev;
		dealloc_node(ptr);
		--_size;
	}
	inline void pop_back() noexcept
	{
		if (ptr_list == NULL)
			return;
		
		linked_list<T>::node endptr = back_node();
		endptr->prev->next = NULL;
		dealloc_node(endptr);
		--_size;
	}
	inline void erase(linked_list<T>::node ptr) noexcept
	{
		if (ptr == NULL || ptr_list == NULL)
			return;
		
		if (ptr == front_node())
			pop_front();
		else if (ptr == back_node())
			pop_back();
		else
			free_node(ptr);
	}
	inline void pop_front() noexcept
	{
		linked_list<T>::node frontptr = front_node();
		if (frontptr->next != NULL) {
			frontptr->next->prev = NULL;
			frontptr = frontptr->next;
			ptr_list = frontptr;
		}
		dealloc_node(ptr_list);
		--_size;
	}
	inline linked_list<T>::node push_back(void) noexcept
	{
		linked_list<T>::node ptr;
		if (ptr_list == NULL) {
			ptr_list = alloc_node();
			ptr = ptr_list;
			ptr_list->prev = NULL;
		}
		else {
			ptr = alloc_node();
			linked_list<T>::node endptr = back_node();
			endptr->next = ptr;
			ptr->prev = endptr;
		}
		ptr->next = NULL;
		++_size;
		return ptr;
	}
	inline linked_list<T>::node emplace_back(void) noexcept
	{ return push_back(); }
	
	// random algos
	inline void swap(linked_list& list)
	{
		if (list.ptr_list == NULL || ptr_list == NULL)
			return;
	}
	inline void swap_nodes(linked_list<T>::iterator iter1, linked_list<T>::iterator iter2) noexcept
	{
		if (ptr_list == NULL || iter1 == NULL || iter2 == NULL)
			return;
		
		if (iter1 == front_node())
			iter1->next->prev = iter2;
		else if (iter1 == back_node())
			iter1->prev->next = iter2;
		else {
			iter1->prev->next = iter2;
			iter1->next->prev = iter2;
		}
		
		if (iter2 == front_node())
			iter2->next->prev = iter1;
		else if (iter2 == back_node())
			iter2->prev->next = iter1;
		else {
			iter2->prev->next = iter1;
			iter2->next->prev = iter1;
		}
	}
	linked_list<T>::node find_node(linked_list<T>::iterator begin_it, std::size_t n,
		const T &val) noexcept
	{
		if (ptr_list == NULL || begin_it == NULL || n == 0)
			return NULL;
		
		for (linked_list<T>::iterator it = begin_it; --n; it = it->next) {
			if (it->val == val) return it;
		}
		return ptr_list;
	}
	inline void erase_n(linked_list<T>::iterator begin_it, std::size_t n) noexcept
	{
		if (begin_it == NULL || ptr_list == NULL || n == 0)
			return;
		
		for (linked_list<T>::iterator it = begin_it; --n; it = it->next)
			erase(it);
	}
	inline void erase_range(linked_list<T>::iterator begin_it, linked_list<T>::iterator end_it) noexcept
	{
		if (begin_it == NULL || end_it == NULL || ptr_list == NULL)
			return;
		
		for (linked_list<T>::iterator it = begin_it; it != end_it; it = it->next)
			erase(it);
	}

	// iterators/access
	inline T& operator[](std::size_t index) noexcept
	{
		if (ptr_list == NULL || index > _size) {
			LOG_WARN("ptr_list is NULL or index > _size in linked_list::operator[]");
			return 0;
		}
		
		linked_list<T>::iterator it = begin();
		while (--index && it != end())
			it = it->next;
		
		return it->val;
	}
	inline std::size_t size(void) const noexcept
	{ return _size; }
	inline linked_list<T>::iterator begin(void) {
		if (ptr_list == NULL)
			LOG_WARN("ptr_list is NULL in linked_list::begin");
		
		return ptr_list;
	}
	inline linked_list<T>::iterator end(void) {
		if (ptr_list == NULL) {
			LOG_WARN("ptr_list is NULL in linked_list::end");
			return NULL;
		}
		
		linked_list<T>::iterator _endptr;
		for (_endptr = begin();; _endptr = _endptr->next) {
			if (_endptr == NULL) break;
		}
		return _endptr;
	}
	inline linked_list<T>::node back_node(void) {
		if (ptr_list == NULL) {
			LOG_WARN("ptr_list is NULL in linked_list::back_node");
			return NULL;
		}
		linked_list<T>::node _endptr;
		for (_endptr = begin();; _endptr = _endptr->next) {
			if (_endptr->next == NULL) break;
		}
		return _endptr;
	}
	inline linked_list<T>::node front_node(void) {
		if (ptr_list == NULL)
			LOG_WARN("ptr_list is NULL in linked_list::front_node");
		
		return ptr_list;
	}
	inline T& back(void) noexcept {
		if (ptr_list == NULL)
			LOG_WARN("ptr_list is NULL in linked_list::back");
		
		return back_node()->val;
    }
	inline T& front(void) noexcept {
		if (ptr_list == NULL)
			LOG_WARN("ptr_list is NULL in linked_list::front");

		return front_node()->val;
	}
};

#endif