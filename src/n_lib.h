#ifndef _N_LIB_
#define _N_LIB_

#pragma once

#if 0
#define BZ_RUN               0
#define BZ_FLUSH             1
#define BZ_FINISH            2

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)
#endif

inline const char* bzip2_strerror(int err)
{
    switch (err) {
    case BZ_CONFIG_ERROR: return "BZ_CONFIG_ERROR, somehow compiled incorrectly";
    case BZ_DATA_ERROR: return "BZ_DATA_ERROR, data integrity whilst decompressing was corrupted";
    case BZ_MEM_ERROR: return "BZ_MEM_ERROR, memory allocation failed to bzip2";
    case BZ_IO_ERROR: return "BZ_IO_ERROR, error reading or writing to compressed file, perhaps check the call stack";
    case BZ_UNEXPECTED_EOF: return "BZ_UNEXPECTED_EOF, EOF bit was returned before logical buffer end was found";
    case BZ_OUTBUFF_FULL: return "BZ_OUTBUF_FULL, buffer overflow occurred";
    case BZ_PARAM_ERROR: return "BZ_PARAM_ERROR, parameter in function call was invalid, check call stack";
    case BZ_SEQUENCE_ERROR: return "BZ_SEQUENCE_ERROR, buggy code has been compiled, this should not happen again";
    case BZ_DATA_ERROR_MAGIC: return "BZ_DATA_ERROR_MAGIC, compressed data buffer did not contain magic bytes of \'BZh\'";
    case BZ_OK: return "No Error.... HOW";
    };
}

#if 0
inline int(*bzip2_bufcompress)(char *, unsigned int *, char *, unsigned int, int, int, int);
inline int(*bzip2_bufdecompress)(char *, unsigned int *, char *, unsigned int, int, int);

#define BZ2_bzBuffToBuffCompress(dst,dstlen,src,srclen,blksize,verbosity,workfactor) (*bzip2_bufcompress)(dst,dstlen,src,srclen,blksize,verbosity,workfactor)
#define BZ2_bzBuffToBuffDecompress(dst,dstlen,src,srclen,small,verbosity) (*bzip2_bufdecompress)(dst,dstlen,src,srclen,small,verbosity)
#endif

#endif