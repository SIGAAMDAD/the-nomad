#ifndef _R_FRAMEVECTOR_
#define _R_FRAMEVECTOR_

#pragma once

template<typename T>
class frameVector
{
public:
    constexpr frameVector(void);
    frameVector(uint64_t size);
    ~frameVector();

    uint64_t size(void) const;
    uint64_t allocated(void) const;
    void set_size(uint64_t size);

    T* data(void);
    const T* data(void) const;

    void reserve(uint64_t size);
    void clear(void);
    
    void push(const T& value);
    template<typename... Args>
    void push(Args&&... args);

    T& operator[](uint32_t index);
    const T& operator[](uint32_t index) const;
private:
    T *m_data;

    uint64_t m_allocated;
    uint64_t m_size;

    void doRealloc(uint64_t addSize)
    {
        m_allocated += addSize * 2;

        T *nptr = (T *)R_FrameAlloc(sizeof(T) * m_allocated);
        if (m_data) {
            memcpy(nptr, m_data, sizeof(T) * m_size);
        }
        m_data = nptr;
    }
};

template<typename T>
constexpr inline frameVector<T>::frameVector(void)
    : m_data(NULL), m_allocated(0), m_size(0)
{
}

template<typename T>
inline frameVector<T>::frameVector(uint64_t size)
{
    m_size = size;
    doRealloc(size);
}

template<typename T>
inline frameVector<T>::~frameVector()
{
    clear();
}

template<typename T>
inline void frameVector<T>::clear(void)
{
    m_size = 0;
    m_allocated = 0;
    m_data = NULL;
}

template<typename T>
inline void frameVector<T>::set_size(uint64_t size)
{
    m_size = size;
}

template<typename T>
inline uint64_t frameVector<T>::size(void) const
{
    return m_size;
}

template<typename T>
inline uint64_t frameVector<T>::allocated(void) const
{
    return m_allocated;
}

template<typename T>
inline T* frameVector<T>::data(void)
{
    return m_data;
}

template<typename T>
inline const T* frameVector<T>::data(void) const
{
    return m_data;
}

template<typename T>
inline void frameVector<T>::reserve(uint64_t size)
{
    doRealloc(size);
}

template<typename T>
inline void frameVector<T>::push(const T& value)
{
    if (m_size + 1 >= m_allocated)
        doRealloc(1);
    
    ::new ((void *)&m_data[m_size]) T(value);
    ++m_size;
}

template<typename T>
template<typename... Args>
inline void frameVector<T>::push(Args&&... args)
{
    if (m_size + 1 >= m_allocated)
        doRealloc(1);
    
    ::new ((void *)&m_data[m_size]) T(eastl::forward<Args>(args)...);
    ++m_size;
}

template<typename T>
inline T& frameVector<T>::operator[](uint32_t index)
{
    return m_data[index];
}

template<typename T>
inline const T& frameVector<T>::operator[](uint32_t index) const
{
    return m_data[index];
}

#endif