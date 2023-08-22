#ifndef _N_LIB_
#define _N_LIB_

#pragma once

#ifdef _WIN32
#define BZIP2_LIB "libbz2.dll"
#define ZLIB_LIB "libz.dll"
#else
#define BZIP2_LIB "./libbz2.so"
#define ZLIB_LIB "./libz.so"
#endif

#endif