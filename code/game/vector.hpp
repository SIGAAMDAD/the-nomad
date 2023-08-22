#ifndef _GDR_VECTOR_
#define _GDR_VECTOR_

#pragma once

constexpr uint32_t allocSize = 2;

typedef int cmp_t(const void *, const void *);

template<typename T>
class GDRVectorBase
{
	typedef GDRVectorBase<T> this_type;
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef uint64_t size_type;
	
	typedef pointer iterator;
	typedef const_pointer const_iterator;
	typedef eastl::reverse_iterator<pointer> reverse_iterator;
	typedef eastl::reverse_iterator<const_pointer> const_reverse_iterator;
public:
	static constexpr size_type vec_begin = 0;
	static constexpr size_type vec_end = (size_type) - 1;
	
	constexpr GDRVectorBase();
	explicit GDRVectorBase(size_type n);
	explicit GDRVectorBase(const_pointer t, size_type size);
	GDRVectorBase(const this_type& vec);
	~GDRVectorBase();

	this_type& operator=(const this_type& other);
	this_type& operator=(std::initializer_list<value_type> ilist);
	
	bool remove(size_type index);
	bool remove(const_reference obj);
	bool cmp(const this_type& vec);
	bool initialized(void) const;
	
	size_type allocated(void) const;
	size_type size(void) const;
	size_type memused(void) const;
	size_type vecsize(void) const;
	constexpr size_type max_size(void) const;
	
	void insert(size_type pos, value_type&& value);
	void copy(const this_type& vec);
	void copyn(const this_type& vec, size_type n);
	void copyn(const_pointer vec, size_type n);
	void resize(size_type nsize);
	void clear(void);
	
	void pop_back(void);
	template<typename... Args>
	reference emplace_back(Args&&... args);
	template<typename... Args>
	reference emplace(const_iterator position, Args&&... args);
	reference push_back(value_type&& value);
	reference push_back(const value_type& value);
	
	void insert(iterator position, value_type&& value);
	void insert(iterator position, std::initializer_list<value_type> ilist);
	void insert(iterator position, const_reference& value);
	void insert(iterator position, size_type n, const_reference& value);
	
	void reserve(size_type amount);
	void sort(cmp_t* cmp);
	void assign(int fill, size_type n);
	void assign(int fill);
	void assign(value_type& value);
	
	reference operator[](size_type index) noexcept;
	reference at(size_type index) noexcept;

	const_reference operator[](size_type index) const noexcept;
	const_reference at(size_type index) const noexcept;
	
	reference front(void) noexcept;
	reference back(void) noexcept;
	const_reference front(void) const noexcept;
	const_reference back(void) const noexcept;
	
	
	pointer find(const_reference obj, size_type* index = NULL);
	pointer data(void) noexcept;
	const_pointer data(void) const noexcept;
	
	iterator begin(void) noexcept;
	iterator end(void) noexcept;
	const_iterator cbegin(void) const noexcept;
	const_iterator cend(void) const noexcept;
	reverse_iterator rbegin(void) noexcept;
	reverse_iterator rend(void) noexcept;
	const_reverse_iterator crbegin(void) const noexcept;
	const_reverse_iterator crend(void) const noexcept;
private:
	size_type m_size;
	size_type m_allocated;
	pointer m_data;
	
	void doRealloc(size_type nsize)
	{
		m_allocated += nsize << allocSize;
		
		pointer nbuf = (pointer)Mem_Alloc(m_allocated * sizeof(value_type));
		if (m_data) {
			memcpy(nbuf, m_data, m_size);
			Mem_Free(m_data);
		}
		m_data = nbuf;
	}
};


template<typename T>
inline constexpr GDRVectorBase<T>::GDRVectorBase()
{
	m_size = 0;
	m_allocated = 0;
	m_data = NULL;
}

template<typename T>
inline GDRVectorBase<T>::GDRVectorBase(size_type n)
{
	m_size = n;
	m_allocated = n + allocSize;
	m_data = (pointer)Mem_Alloc(sizeof(value_type) * m_allocated);
	memset(m_data, 0, sizeof(value_type) * m_size);
}

template<typename T>
inline GDRVectorBase<T>::GDRVectorBase(const_pointer t, size_type size)
{
	m_size = size;
	m_allocated = size + allocSize;
	m_data = (pointer)Mem_Alloc(sizeof(value_type) * m_allocated);
	memcpy(m_data, t, size);
}

template<typename T>
inline GDRVectorBase<T>::GDRVectorBase(const this_type& vec)
{
	m_size = vec.m_size;
	m_allocated = vec.m_allocated;
	m_data = (pointer)Mem_Alloc(sizeof(value_type) * m_allocated);
	memcpy(m_data, vec.m_data, sizeof(value_type) * m_size);
}

template<typename T>
inline GDRVectorBase<T>::~GDRVectorBase()
{
	clear();
}

template<typename T>
inline bool GDRVectorBase<T>::remove(size_type index)
{
	size_type i;
	if (!m_data || !m_size) {
		return false;
	}
	if ((index < 0) || (index >= m_size)) {
		return false;
	}
	
	m_size--;
	for (i = index; i < m_size; ++i) {
		m_data[i] = m_data[i+1];
	}
	
	return true;
}

template<typename T>
inline bool GDRVectorBase<T>::remove(const_reference obj)
{
	size_type index;
	return find(obj, &index) != NULL ? remove(index) : false;
}

template<typename T>
inline bool GDRVectorBase<T>::cmp(const this_type& vec)
{
}

template<typename T>
inline bool GDRVectorBase<T>::initialized(void) const
{
	return m_allocated > 0 ? true : false;
}	

template<typename T>
inline typename GDRVectorBase<T>::size_type GDRVectorBase<T>::allocated(void) const
{
	return m_allocated;
}

template<typename T>
inline typename GDRVectorBase<T>::size_type GDRVectorBase<T>::size(void) const
{
	return m_size;
}

template<typename T>
inline typename GDRVectorBase<T>::size_type GDRVectorBase<T>::memused(void) const
{
	return m_allocated * sizeof(value_type);
}

template<typename T>
inline typename GDRVectorBase<T>::size_type GDRVectorBase<T>::vecsize(void) const
{
	return m_size * sizeof(value_type);
}

template<typename T>
inline constexpr typename GDRVectorBase<T>::size_type GDRVectorBase<T>::max_size(void) const
{
	return eastl::numeric_limits<ptrdiff_t>::max() / sizeof(value_type);
}

template<typename T>
inline void GDRVectorBase<T>::insert(size_type pos, value_type&& value)
{
	insert(&m_data[pos], eastl::move(value));
}

template<typename T>
inline void GDRVectorBase<T>::copy(const this_type& vec)
{
	if (m_size == vec.m_size) {
        memcpy(m_data, vec.m_data, sizeof(value_type) * m_size);
        return;
    }
    if (m_size > vec.m_size) {
        m_size = vec.m_size;
        memcpy(m_data, vec.m_data, sizeof(value_type) * m_size);
        return;
    }
	clear();
	doRealloc(vec.m_size);
	m_size = vec.m_size;
	memcpy(m_data, vec.m_data, sizeof(value_type) * vec.m_size);
}

template<typename T>
inline void GDRVectorBase<T>::copyn(const this_type& vec, size_type n)
{
	// quick error checks
	if (!n) {
		return;
	}
	if (vec.m_size < n) {
		return;
	}
	
	// simple copy
	if (m_size == n) {
		memcpy(m_data, vec.m_data, sizeof(value_type) * n);
		return;
	}
	// no need for reallocation
	if (m_size > n) {
		m_allocated = m_size - n;
		m_size = n;
		memcpy(m_data, vec.m_data, sizeof(value_type) * n);
		return;
	}
	// slowest of them all
	if (m_size < n) {
		clear();
		doRealloc(n);
		m_size = n;
		memcpy(m_data, vec.m_data, sizeof(value_type) * n);
		return;
	}
}

template<typename T>
inline void GDRVectorBase<T>::copyn(const_pointer vec, size_type n)
{
	// quick error checking
	if (!vec) {
		return;
	}
	if (!n) {
		return;
	}
	
	// simple copy
	if (m_size == n) {
		memcpy(m_data, vec, sizeof(value_type) * n);
		return;
	}
	// no need for reallocation
	if (m_size > n) {
		m_allocated = m_size - n;
		m_size = n;
		memcpy(m_data, vec, sizeof(value_type) * n);
		return;
	}
	// slowest of them all
	if (m_size < n) {
		clear();
		doRealloc(n);
		m_size = n;
		memcpy(m_data, vec, sizeof(value_type) * n);
		return;
	}
}


template<typename T>
inline void GDRVectorBase<T>::resize(size_type nsize)
{
	if (!nsize) {
		clear();
		return;
	}
	if (m_size == nsize) {
		return;
	}
	
	pointer temp = m_data;
	m_allocated = nsize * 2;
	m_size = nsize;
	
	m_data = (pointer)Mem_Alloc(sizeof(value_type) * m_allocated);
	memcpy(m_data, temp, m_size);
	
	if (temp) {
		Mem_Free(temp);
	}
}

template<typename T>
inline void GDRVectorBase<T>::clear(void)
{
	if (m_data)
		Mem_Free(m_data);
	
	m_size = 0;
	m_allocated = 0;
	m_data = NULL;
}

template<typename T>
inline void GDRVectorBase<T>::pop_back(void)
{
	if (!m_size) {
		return;
	}
	// no need to deallocate
	memset(&m_data[m_size], 0, sizeof(value_type));
	m_size--;
}

template<typename T>
template<typename... Args>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::emplace_back(Args&&... args)
{
	if (!m_data || m_size + 1 >= m_allocated) {
		doRealloc(m_size + 1);
	}
	
	// construct the object
	::new ((void *)&m_data[m_size]) value_type(eastl::forward<Args>(args)...);
	m_size++;
	return back();
}

template<typename T>
template<typename... Args>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::emplace(const_iterator position, Args&&... args)
{
	if (!m_data || m_size + 1 >= m_allocated) {

	}
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::push_back(value_type&& value)
{
	return emplace_back(eastl::move(value));
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::push_back(const value_type& value)
{
	return emplace_back(eastl::move(value));
}

template<typename T>
inline void GDRVectorBase<T>::insert(iterator position, value_type&& value)
{
	if (end() < position) {
		return;
	}
	doRealloc(m_size + 1);
	
	for (size_type i = 0; i < m_size; ++i) {
		m_data[pos + 1] = m_data[pos];
	}
	::new((void *)position) value_type(eastl::move(value));
	m_size++;
}

template<typename T>
inline void GDRVectorBase<T>::insert(iterator position, std::initializer_list<value_type> ilist)
{

}

template<typename T>
inline void GDRVectorBase<T>::insert(iterator position, const value_type& value)
{

}

template<typename T>
inline void GDRVectorBase<T>::insert(iterator position, size_type n, const value_type& value)
{

}

template<typename T>
inline void GDRVectorBase<T>::reserve(size_type amount)
{
	if (m_size + amount >= m_allocated) {
		doRealloc(m_size + amount);
	}
	m_size += amount;
}

template<typename T>
inline void GDRVectorBase<T>::sort(cmp_t* cmp)
{
	if (!m_data || !cmp) {
		return;
	}
	
	qsort((void *)m_data, (size_t)m_size, sizeof(value_type), cmp);
}

template<typename T>
inline void GDRVectorBase<T>::assign(int fill, size_type n)
{
	if (n > m_size)
		return;

	memset(m_data, fill, n);
}

template<typename T>
inline void GDRVectorBase<T>::assign(int fill)
{
	memset(m_data, fill, m_size);
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::operator[](size_type index) noexcept
{
	return m_data[index];
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::at(size_type index) noexcept
{
	return m_data[index];
}

template<typename T>
inline typename GDRVectorBase<T>::const_reference GDRVectorBase<T>::operator[](size_type index) const noexcept
{
	return m_data[index];
}

template<typename T>
inline typename GDRVectorBase<T>::const_reference GDRVectorBase<T>::at(size_type index) const noexcept
{
	return m_data[index];
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::front(void) noexcept
{
	return *m_data;
}

template<typename T>
inline typename GDRVectorBase<T>::reference GDRVectorBase<T>::back(void) noexcept
{
	return m_data[m_size];
}

template<typename T>
inline typename GDRVectorBase<T>::const_reference GDRVectorBase<T>::front(void) const noexcept
{
	return *m_data;
}

template<typename T>
inline typename GDRVectorBase<T>::const_reference GDRVectorBase<T>::back(void) const noexcept
{
	return m_data[m_size];
}

template<typename T>
inline typename GDRVectorBase<T>::pointer GDRVectorBase<T>::find(const_reference obj, size_type* index)
{
	for (size_type i = 0; i < m_size; ++i) {
		if (m_data[i] == obj) {
			if (index) {
				*index = i;
			}
			return &m_data[i];
		}
	}
	return NULL;
}

template<typename T>
inline typename GDRVectorBase<T>::pointer GDRVectorBase<T>::data(void) noexcept
{
	return m_data;
}

template<typename T>
inline typename GDRVectorBase<T>::const_pointer GDRVectorBase<T>::data(void) const noexcept
{
	return m_data;
}

template<typename T>
inline typename GDRVectorBase<T>::iterator GDRVectorBase<T>::begin(void) noexcept
{
	return iterator(m_data);
}

template<typename T>
inline typename GDRVectorBase<T>::iterator GDRVectorBase<T>::end(void) noexcept
{
	return iterator(m_data + m_size);
}

template<typename T>
inline typename GDRVectorBase<T>::const_iterator GDRVectorBase<T>::cbegin(void) const noexcept
{
	return const_iterator(m_data);
}

template<typename T>
inline typename GDRVectorBase<T>::const_iterator GDRVectorBase<T>::cend(void) const noexcept
{
	return const_iterator(m_data + m_size);
}

template<typename T>
inline typename GDRVectorBase<T>::reverse_iterator GDRVectorBase<T>::rbegin(void) noexcept
{
	return reverse_iterator(m_data + m_size);
}

template<typename T>
inline typename GDRVectorBase<T>::reverse_iterator GDRVectorBase<T>::rend(void) noexcept
{
	return reverse_iterator(m_data);
}

template<typename T>
inline typename GDRVectorBase<T>::const_reverse_iterator GDRVectorBase<T>::crbegin(void) const noexcept
{
	return const_reverse_iterator(m_data + m_size);
}

template<typename T>
inline typename GDRVectorBase<T>::const_reverse_iterator GDRVectorBase<T>::crend(void) const noexcept
{
	return const_reverse_iterator(m_data);
}

// the basic vector type
template<typename T>
using GDRVector = GDRVectorBase<T>;

#endif