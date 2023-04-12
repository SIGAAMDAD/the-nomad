#ifndef _N_LIB_
#define _N_LIB_

#pragma once

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


inline int(*bzip2_bufcompress)(char *, unsigned int *, char *, unsigned int, int, int, int);
inline int(*bzip2_bufdecompress)(char *, unsigned int *, char *, unsigned int, int, int);

#define BZ2_bzBuffToBuffCompress(dst,dstlen,src,srclen,blksize,verbosity,workfactor) (*bzip2_bufcompress)(dst,dstlen,src,srclen,blksize,verbosity,workfactor)
#define BZ2_bzBuffToBuffDecompress(dst,dstlen,src,srclen,small,verbosity) (*bzip2_bufdecompress)(dst,dstlen,src,srclen,small,verbosity)

enum
{	/* Major formats. */
	SF_FORMAT_WAV			= 0x010000,		/* Microsoft WAV format (little endian default). */
	SF_FORMAT_AIFF			= 0x020000,		/* Apple/SGI AIFF format (big endian). */
	SF_FORMAT_AU			= 0x030000,		/* Sun/NeXT AU format (big endian). */
	SF_FORMAT_RAW			= 0x040000,		/* RAW PCM data. */
	SF_FORMAT_PAF			= 0x050000,		/* Ensoniq PARIS file format. */
	SF_FORMAT_SVX			= 0x060000,		/* Amiga IFF / SVX8 / SV16 format. */
	SF_FORMAT_NIST			= 0x070000,		/* Sphere NIST format. */
	SF_FORMAT_VOC			= 0x080000,		/* VOC files. */
	SF_FORMAT_IRCAM			= 0x0A0000,		/* Berkeley/IRCAM/CARL */
	SF_FORMAT_W64			= 0x0B0000,		/* Sonic Foundry's 64 bit RIFF/WAV */
	SF_FORMAT_MAT4			= 0x0C0000,		/* Matlab (tm) V4.2 / GNU Octave 2.0 */
	SF_FORMAT_MAT5			= 0x0D0000,		/* Matlab (tm) V5.0 / GNU Octave 2.1 */
	SF_FORMAT_PVF			= 0x0E0000,		/* Portable Voice Format */
	SF_FORMAT_XI			= 0x0F0000,		/* Fasttracker 2 Extended Instrument */
	SF_FORMAT_HTK			= 0x100000,		/* HMM Tool Kit format */
	SF_FORMAT_SDS			= 0x110000,		/* Midi Sample Dump Standard */
	SF_FORMAT_AVR			= 0x120000,		/* Audio Visual Research */
	SF_FORMAT_WAVEX			= 0x130000,		/* MS WAVE with WAVEFORMATEX */
	SF_FORMAT_SD2			= 0x160000,		/* Sound Designer 2 */
	SF_FORMAT_FLAC			= 0x170000,		/* FLAC lossless file format */
	SF_FORMAT_CAF			= 0x180000,		/* Core Audio File format */
	SF_FORMAT_WVE			= 0x190000,		/* Psion WVE format */
	SF_FORMAT_OGG			= 0x200000,		/* Xiph OGG container */
	SF_FORMAT_MPC2K			= 0x210000,		/* Akai MPC 2000 sampler */
	SF_FORMAT_RF64			= 0x220000,		/* RF64 WAV file */
	SF_FORMAT_MPEG			= 0x230000,		/* MPEG-1/2 audio stream */

	/* Subtypes from here on. */

	SF_FORMAT_PCM_S8		= 0x0001,		/* Signed 8 bit data */
	SF_FORMAT_PCM_16		= 0x0002,		/* Signed 16 bit data */
	SF_FORMAT_PCM_24		= 0x0003,		/* Signed 24 bit data */
	SF_FORMAT_PCM_32		= 0x0004,		/* Signed 32 bit data */

	SF_FORMAT_PCM_U8		= 0x0005,		/* Unsigned 8 bit data (WAV and RAW only) */

	SF_FORMAT_FLOAT			= 0x0006,		/* 32 bit float data */
	SF_FORMAT_DOUBLE		= 0x0007,		/* 64 bit float data */

	SF_FORMAT_ULAW			= 0x0010,		/* U-Law encoded. */
	SF_FORMAT_ALAW			= 0x0011,		/* A-Law encoded. */
	SF_FORMAT_IMA_ADPCM		= 0x0012,		/* IMA ADPCM. */
	SF_FORMAT_MS_ADPCM		= 0x0013,		/* Microsoft ADPCM. */

	SF_FORMAT_GSM610		= 0x0020,		/* GSM 6.10 encoding. */
	SF_FORMAT_VOX_ADPCM		= 0x0021,		/* OKI / Dialogix ADPCM */

	SF_FORMAT_NMS_ADPCM_16	= 0x0022,		/* 16kbs NMS G721-variant encoding. */
	SF_FORMAT_NMS_ADPCM_24	= 0x0023,		/* 24kbs NMS G721-variant encoding. */
	SF_FORMAT_NMS_ADPCM_32	= 0x0024,		/* 32kbs NMS G721-variant encoding. */

	SF_FORMAT_G721_32		= 0x0030,		/* 32kbs G721 ADPCM encoding. */
	SF_FORMAT_G723_24		= 0x0031,		/* 24kbs G723 ADPCM encoding. */
	SF_FORMAT_G723_40		= 0x0032,		/* 40kbs G723 ADPCM encoding. */

	SF_FORMAT_DWVW_12		= 0x0040, 		/* 12 bit Delta Width Variable Word encoding. */
	SF_FORMAT_DWVW_16		= 0x0041, 		/* 16 bit Delta Width Variable Word encoding. */
	SF_FORMAT_DWVW_24		= 0x0042, 		/* 24 bit Delta Width Variable Word encoding. */
	SF_FORMAT_DWVW_N		= 0x0043, 		/* N bit Delta Width Variable Word encoding. */

	SF_FORMAT_DPCM_8		= 0x0050,		/* 8 bit differential PCM (XI only) */
	SF_FORMAT_DPCM_16		= 0x0051,		/* 16 bit differential PCM (XI only) */

	SF_FORMAT_VORBIS		= 0x0060,		/* Xiph Vorbis encoding. */
	SF_FORMAT_OPUS			= 0x0064,		/* Xiph/Skype Opus encoding. */

	SF_FORMAT_ALAC_16		= 0x0070,		/* Apple Lossless Audio Codec (16 bit). */
	SF_FORMAT_ALAC_20		= 0x0071,		/* Apple Lossless Audio Codec (20 bit). */
	SF_FORMAT_ALAC_24		= 0x0072,		/* Apple Lossless Audio Codec (24 bit). */
	SF_FORMAT_ALAC_32		= 0x0073,		/* Apple Lossless Audio Codec (32 bit). */

	SF_FORMAT_MPEG_LAYER_I	= 0x0080,		/* MPEG-1 Audio Layer I */
	SF_FORMAT_MPEG_LAYER_II	= 0x0081,		/* MPEG-1 Audio Layer II */
	SF_FORMAT_MPEG_LAYER_III = 0x0082,		/* MPEG-2 Audio Layer III */

	/* Endian-ness options. */

	SF_ENDIAN_FILE			= 0x00000000,	/* Default file endian-ness. */
	SF_ENDIAN_LITTLE		= 0x10000000,	/* Force little endian-ness. */
	SF_ENDIAN_BIG			= 0x20000000,	/* Force big endian-ness. */
	SF_ENDIAN_CPU			= 0x30000000,	/* Force CPU endian-ness. */

	SF_FORMAT_SUBMASK		= 0x0000FFFF,
	SF_FORMAT_TYPEMASK		= 0x0FFF0000,
	SF_FORMAT_ENDMASK		= 0x30000000
};

enum
{	/* True and false */
	SF_FALSE	= 0,
	SF_TRUE		= 1,

	/* Modes for opening files. */
	SFM_READ	= 0x10,
	SFM_WRITE	= 0x20,
	SFM_RDWR	= 0x30,

	SF_AMBISONIC_NONE		= 0x40,
	SF_AMBISONIC_B_FORMAT	= 0x41
};

typedef	struct sf_private_tag	SNDFILE;
typedef int64_t			sf_count_t;

struct SF_INFO
{	sf_count_t	frames ;		/* Used to be called samples.  Changed to avoid confusion. */
	int			samplerate ;
	int			channels ;
	int			format ;
	int			sections ;
	int			seekable ;
};

typedef	struct SF_INFO SF_INFO;

inline SNDFILE*(*sndfile_open)(const char *, int, SF_INFO *);
inline sf_count_t(*sndfile_readshort)(SNDFILE *, short *, sf_count_t);
inline void(*sndfile_close)(SNDFILE *);

#define sf_open(name,flags,fdata) (*sndfile_open)(name,flags,fdata)
#define sf_read_short(file,buffer,count) (*sndfile_readshort)(file,buffer,count)
#define sf_close(file) (*sndfile_close)(file)

#endif