#ifndef _GDR_STRING_
#define _GDR_STRING_

#pragma once

#include "z_heap.h"

#ifdef __cplusplus

constexpr uint32_t BASE_CHAR_BUFFER_SIZE = 256;
constexpr uint32_t STRING_GROW_SIZE = 256;

// change the buffer size depending how how much stack should be consumed before heap allocation
#ifndef bufferSize
#define bufferSize BASE_CHAR_BUFFER_SIZE
#endif

class GDRStr
{
private:
	char *buffer;
	char *buf_p;
	char basebuffer[bufferSize];
	uint64_t heapsize;
	uint64_t len; // not including null char
	
	void checkAlloc(uint64_t newstr)
	{
		buf_p = basebuffer;
		if (newstr >= bufferSize && newstr >= heapsize) {
			heapsize = newstr + STRING_GROW_SIZE + 1;
			char *newbuf = (char *)Mem_Alloc(heapsize);
			if (buffer) {
				N_strncpyz(newbuf, buffer, len);
				Mem_Free(buffer);
			}
			buffer = newbuf;
			buf_p = buffer;
		}
	}
public:
	static const uint64_t npos = (uint64_t) - 1;

	constexpr GDRStr();
	explicit GDRStr(const GDRStr& str, uint64_t start, uint64_t end);
	explicit GDRStr(const char *__restrict str, uint64_t start, uint64_t end);
	GDRStr(const GDRStr& str);
	GDRStr(const char *__restrict str);
	GDRStr(const bool b);
	GDRStr(const double d);
	GDRStr(const int i);
	~GDRStr();
	
	const char* c_str(void) const;
	char* data(void);
	
	uint64_t find(const char c) const;

	int cmp(const char *__restrict str);
	int cmpn(const char *__restrict str, uint64_t n);
	int casecmp(const char *__restrict str);
	int casecmpn(const char *__restrict str, uint64_t n);
	int cmp(const GDRStr& str);
	int cmpn(const GDRStr& str, uint64_t n);
	int casecmp(const GDRStr& str);
	int casecmpn(const GDRStr& str, uint64_t n);

	void tolower(void);
	void toupper(void);

	char operator[](uint64_t index) const;
	char& operator[](uint64_t index);
	operator const char *(void) const;
	operator const char *(void);
	GDRStr& operator=(const char *__restrict str);
	GDRStr& operator=(const GDRStr& str);
	bool operator==(const char *__restrict str) const;
	bool operator==(const GDRStr& str) const;
	
	GDRStr& operator+=(const char c);
	GDRStr& operator+=(const bool b);
	GDRStr& operator+=(const double d);
	GDRStr& operator+=(const float f);
	GDRStr& operator+=(const short s);
	GDRStr& operator+=(const int i);
	GDRStr& operator+=(const char *__restrict str);
	GDRStr& operator+=(const GDRStr& str);
	
	friend GDRStr operator+(const GDRStr& str1, const GDRStr& str2);
	friend GDRStr operator+(const GDRStr& str1, const char *__restrict str2);
	friend GDRStr operator+(const char *__restrict str1, const GDRStr& str2);
	
	uint64_t length(void) const;
	uint64_t size(void) const;
	uint64_t allocated(void) const;
	
	void resize(uint64_t nsize);
	void clear(void);
	
	void append(const char c);
	void append(const bool b);
	void append(const double d);
	void append(const float f);
	void append(const short s);
	void append(const int i);
	void append(const char *__restrict str);
	void append(const char *__restrict str, uint64_t n);
	void append(const GDRStr& str);
	void append(const GDRStr& str, uint64_t n);
	
	void copy(const char *__restrict str);
	void copyn(const char *__restrict str, uint64_t n);
	void copy(const GDRStr& str);
	void copyn(const GDRStr& str, uint64_t n);
};

inline GDRStr::GDRStr(const GDRStr& str, uint64_t start, uint64_t end)
{
	buffer = (char *)Mem_Alloc(start - end + STRING_GROW_SIZE);
	N_strncpyz(buffer, str.buffer, start - end);
	len = start - end;
	heapsize = start - end + STRING_GROW_SIZE;
}

inline GDRStr::GDRStr(const char *__restrict str, uint64_t start, uint64_t end)
{
	buffer = (char *)Mem_Alloc(start - end + STRING_GROW_SIZE);
	N_strncpyz(buffer, str, end - start);
}

constexpr inline GDRStr::GDRStr()
	: buffer(NULL), buf_p(basebuffer), basebuffer{0},
	heapsize(0), len(0)
{
}

inline GDRStr::GDRStr(const GDRStr& str)
{
	len = str.len;
	heapsize = str.heapsize;
	buffer = NULL;
	checkAlloc(str.len);
	N_strncpy(buf_p, str.buf_p, len);
	buf_p[len - 1] = '\0';
}

inline GDRStr::GDRStr(const char *__restrict str)
{
	memset(basebuffer, 0, sizeof(basebuffer));
	if (str) {
		len = strlen(str);
		heapsize = 0;
		buffer = NULL;
		checkAlloc(len);
		N_strncpy(buf_p, str, len);
		buf_p[len - 1] = '\0';
	}
}

inline GDRStr::~GDRStr()
{
	clear();
}

inline bool GDRStr::operator==(const GDRStr& str) const
{
	return N_strcmp(buf_p, str.buf_p) == 1 ? true : false;
}

inline bool GDRStr::operator==(const char *__restrict str) const
{
	return N_strcmp(buf_p, str) == 1 ? true : false;
}

inline int GDRStr::cmp(const char *__restrict str)
{
	return N_strcmp(buf_p, str);
}

inline int GDRStr::cmpn(const char *__restrict str, uint64_t n)
{
	return N_strncmp(buf_p, str, n);
}

inline int GDRStr::cmp(const GDRStr& str)
{
	return N_strcmp(buf_p, str.buf_p);
}

inline int GDRStr::casecmp(const char *__restrict str)
{
	return N_stricmp(buf_p, str);
}

inline int GDRStr::casecmpn(const char *__restrict str, uint64_t n)
{
	return N_stricmpn(buf_p, str, n);
}

inline int GDRStr::casecmp(const GDRStr& str)
{
	return N_stricmp(buf_p, str.buf_p);
}

inline int GDRStr::casecmpn(const GDRStr& str, uint64_t n)
{
	return N_stricmpn(buf_p, str.buf_p, n);
}

inline void GDRStr::tolower(void)
{
	for (uint64_t i = 0; i < len; ++i) {
		buf_p[i] = ::tolower(buf_p[i]);
	}
}

inline void GDRStr::toupper(void)
{
	for (uint64_t i = 0; i < len; ++i) {
		buf_p[i] = ::toupper(buf_p[i]);
	}
}

inline void GDRStr::copy(const char *__restrict str)
{
	checkAlloc(strlen(str));
	N_strncpyz(buf_p, str, strlen(str));
	len = strlen(str);
}

inline void GDRStr::copyn(const char *__restrict str, uint64_t n)
{
	if (str && n) {
		checkAlloc(n);
		N_strncpyz(buf_p, str, n);
		len = n;
	}
}

inline void GDRStr::copy(const GDRStr& str)
{
	checkAlloc(str.len);
	N_strncpyz(buf_p, str.buf_p, str.len);
	len = str.len;
}

inline void GDRStr::copyn(const GDRStr& str, uint64_t n)
{
	checkAlloc(n);
	N_strncpyz(buf_p, str.buf_p, n);
	len = n;
}

inline void GDRStr::resize(uint64_t nsize)
{
	checkAlloc(nsize);
	buf_p[nsize] = '\0';
	len = nsize;
}

inline void GDRStr::clear(void)
{
	if (buffer) {
		Mem_Free(buffer);
	}
	len = 0;
	heapsize = 0;
	memset(basebuffer, 0, sizeof(basebuffer));
}

inline const char* GDRStr::c_str(void) const
{
	return buf_p;
}

inline uint64_t GDRStr::find(const char c) const
{
	uint64_t pos = npos;
	for (uint64_t i = 0; i < len; ++i) {
		if (buf_p[i] == c) {
			pos = i;
			break;
		}
	}
	return pos;
}

inline char* GDRStr::data(void)
{
	return buf_p;
}

inline uint64_t GDRStr::length(void) const
{
	return len;
}

inline uint64_t GDRStr::size(void) const
{
	return len;
}

inline uint64_t GDRStr::allocated(void) const
{
	return heapsize;
}

inline void GDRStr::append(const char *__restrict str)
{
	if (str) {
		uint64_t newlen = strlen(str) + len;
		
		checkAlloc(newlen);
		for (uint64_t i = 0; str[i] && i < newlen; i++) {
			buf_p[len + i] = str[i];
		}
		len = newlen;
		buf_p[len] = '\0';
	}
}

inline void GDRStr::append(const char *__restrict str, uint64_t n)
{
	if (str && n) {
		uint64_t newlen = len + n;
		
		checkAlloc(newlen);
		for (uint64_t i = 0; str[i] && i < n; i++) {
			buf_p[len + i] = str[i];
		}
		len = newlen;
		buf_p[len] = '\0';
	}
}

inline void GDRStr::append(const GDRStr& str)
{
	append(str.buf_p, str.len);
}

inline void GDRStr::append(const GDRStr& str, size_t n)
{
	append(str.buf_p, n);
}

inline char GDRStr::operator[](uint64_t index) const
{
    return buf_p[index];
}

inline char& GDRStr::operator[](uint64_t index)
{
    return buf_p[index];
}

inline GDRStr::operator const char *(void) const
{
	return c_str();
}

inline GDRStr::operator const char *(void)
{
	return c_str();
}

inline GDRStr& GDRStr::operator=(const GDRStr& str)
{
	return GDRStr::operator=(str.buf_p);
}

inline void GDRStr::append(const bool b)
{
	char buf[12];
	sprintf(buf, "%s", (b ? "true" : "false"));
	append(buf, 11);
}

inline void GDRStr::append(const char c)
{
	checkAlloc(len+1);
	buf_p[len] = c;
	len++;
	buf_p[len] = '\0';
	len++;
}

inline void GDRStr::append(const short s)
{
	char buf[64];
	sprintf(buf, "%hi", s);
	append(buf);
}

inline void GDRStr::append(const int i)
{
	char buf[64];
	sprintf(buf, "%i", i);
	append(buf);
}

inline void GDRStr::append(const double d)
{
	char buf[64];
	sprintf(buf, "%f", d);
	append(buf);
}

inline void GDRStr::append(const float f)
{
	char buf[64];
	sprintf(buf, "%f", f);
	append(buf);
}


inline GDRStr& GDRStr::operator=(const char *__restrict str)
{
	if (!str) {
		checkAlloc(1);
		buf_p[0] = '\0';
		len = 0;
		return *this;
	}
	if (buf_p == str) {
		return *this;
	}
	if (str >= buf_p && str <= buf_p + len) {
		const ptrdiff_t diff = str - buf_p;

		assert(strlen(str) < (unsigned)len);
		uint64_t i;
		for (i = 0; str[i]; ++i) {
			buf_p[i] = str[i];
		}
		buf_p[i] = '\0';
		len -= diff;

		return *this;
	}

	uint64_t slen = strlen(str);
	checkAlloc(slen);
	N_strncpyz(buf_p, str, slen);
	buf_p[slen] = '\0';
	len = slen;
	return *this;
}

inline GDRStr& GDRStr::operator+=(const char c)
{
	checkAlloc(len+sizeof(c));
	append(c);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const bool b)
{
	checkAlloc(len+sizeof(b));
	append(b);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const double d)
{
	checkAlloc(len+sizeof(d));
	append(d);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const float f)
{
	checkAlloc(len+sizeof(f));
	append(f);
	return *this;	
}

inline GDRStr& GDRStr::operator+=(const short s)
{
	checkAlloc(len+sizeof(s));
	append(s);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const int i)
{
	checkAlloc(len+sizeof(i));
	append(i);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const char *__restrict str)
{
	append(str);
	return *this;
}

inline GDRStr& GDRStr::operator+=(const GDRStr& str)
{
	append(str);
	return *this;
}
#endif


#endif
