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
#ifndef _G_ZONE_
#define _G_ZONE_

#pragma once


enum : uint8_t
{
	TAG_FREE       = 0,
	TAG_STATIC     = 1,
	TAG_LEVEL      = 2,
	TAG_SFX        = 3,
	TAG_MUSIC      = 4,
	TAG_PURGELEVEL = 100,
	TAG_CACHE      = 101,
	
	NUMTAGS
};

#define TAG_SCOPE TAG_CACHE
#define TAG_LOAD TAG_CACHE

void* Z_Malloc(size_t size, int tag, void *user);
void* Z_AlignedAlloc(size_t alignment, size_t size, int tag, void *user, const char* name);
void* Z_Malloc(size_t size, int tag, void *user, const char* name);
void* Z_Calloc(void *user, size_t nelem, size_t elemsize, int tag, const char* name);
void* Z_Realloc(void *ptr, size_t nsize, void *user, int tag, const char* name);
void Z_Free(void *ptr);
void Z_FreeTags(int lowtag, int hightag);
void Z_ChangeTag(void* user, int tag);
void Z_ChangeUser(void* olduser, void* newuser);
void Z_ChangeName(void* user, const char* name);
void Z_ClearCache();
void Z_CheckHeap();
void Z_ClearZone();
void Z_Print(bool all);
void Z_Init();
int Z_FreeMemory(void);

void *Z_ZoneBegin(void);
void *Z_ZoneEnd(void);

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

template<class T>
struct zone_allocator
{
	zone_allocator() noexcept { }

	typedef T value_type;
    template<class U>
    constexpr zone_allocator(const zone_allocator<U>&) noexcept { }

    [[nodiscard]] inline T* allocate(std::size_t n) const {
        T* p = NULL;
		if ((p = static_cast<T*>(Z_Malloc(n, TAG_STATIC, &p, "zallocator"))) != NULL)
            return p;

        throw std::bad_alloc();
    }
	[[nodiscard]] inline T* allocate(std::size_t& n, std::size_t& alignment, std::size_t& offset) const {
		T* p = NULL;
		if ((p = static_cast<T*>(Z_Malloc(n, TAG_STATIC, &p, "zallocator"))) != NULL)
			return p;
		
		throw std::bad_alloc();
	}
	void deallocate(T* p, std::size_t n) const noexcept {
		Z_Free((void *)p);
	}
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
			sizeof(linked_list<T>::list_node), TAG_STATIC, &ptr, "listnode");
		if (ptr == NULL)
			N_Error("linked_list::alloc_node: memory allocation failed");
		
		return ptr;
	}
	void dealloc_node(linked_list<T>::node ptr) noexcept {
		if (ptr == NULL)
			return;
		Z_Free((void *)ptr);
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
			Con_Printf("WARNING: ptr_list is NULL or index > _size in linked_list::operator[]");
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
			Con_Printf("WARNING: ptr_list is NULL in linked_list::begin");
		
		return ptr_list;
	}
	inline linked_list<T>::iterator end(void) {
		if (ptr_list == NULL) {
			Con_Printf("WARNING: ptr_list is NULL in linked_list::end");
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
			Con_Printf("WARNING: ptr_list is NULL in linked_list::back_node");
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
			Con_Printf("WARNING: ptr_list is NULL in linked_list::front_node");
		
		return ptr_list;
	}
	inline T& back(void) noexcept {
		if (ptr_list == NULL)
			Con_Printf("WARNING: ptr_list is NULL in linked_list::back");
		
		return back_node()->val;
    }
	inline T& front(void) noexcept {
		if (ptr_list == NULL)
			Con_Printf("WARNING: ptr_list is NULL in linked_list::front");

		return front_node()->val;
	}
};

#endif