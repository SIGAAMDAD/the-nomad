#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include <SDL2/SDL.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/mman.h>

qboolean Sys_RandomBytes( byte *s, uint64_t len )
{
    FILE *fp;

    fp = fopen("/dev/urandom", "r");
    if (!fp)
        return qfalse;
    
    setvbuf(fp, NULL, _IONBF, 0); // don't buffer reads from /dev/urandom

    if (fread(s, sizeof(byte), len, fp) != len) {
        fclose(fp);
        return qfalse;
    }

    fclose(fp);
    return qtrue;
}

void Sys_Sleep( double msec ) {
    usleep( msec * 1000.0f );
}

/*
* Sys_StackMemoryRemaining: returns the maximum amount of process stack memory
*/
uint64_t Sys_StackMemoryRemaining( void )
{
    struct rlimit limit;
    getrlimit( RLIMIT_STACK, &limit );
    return limit.rlim_cur;
}

void Sys_ListFilteredFiles(const char *basedir, const char *subdirs, const char *filter, char **list, uint64_t *numfiles)
{
    char search[MAX_OSPATH*2+1];
    char newsubdirs[MAX_OSPATH*2];
    char filename[MAX_OSPATH*2];
    DIR *fdir;
    struct dirent *d;
    struct stat st;

    if (*numfiles >= MAX_FOUND_FILES - 1)
        return;

    if (*subdirs)
        Com_snprintf(search, sizeof(search), "%s/%s", basedir, subdirs);
    else
        Com_snprintf(search, sizeof(search), "%s", basedir);
    
    if ((fdir = opendir(search)) == NULL)
        return;
    
    while ((d = readdir(fdir)) != NULL) {
        Com_snprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
        if (stat(filename, &st) == -1)
            continue;
        
        if (st.st_mode & S_IFDIR) {
            if ( d->d_name[0] == '.' || ( d->d_name[0] == '.' && d->d_name[1] == '.' ) ) {
                continue;
            }
            if (*subdirs)
                Com_snprintf(newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
            else
                Com_snprintf(newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
            
            Sys_ListFilteredFiles(basedir, newsubdirs, filter, list, numfiles);
        }
        if (*numfiles >= MAX_FOUND_FILES - 1)
            break;

        Com_snprintf(filename, sizeof(filename), "%s/%s", subdirs, d->d_name);
        if (!Com_FilterPath(filter, filename))
            continue;
        
        list[*numfiles] = FS_CopyString(filename);
        (*numfiles)++;
    }
    closedir(fdir);
}

char **Sys_ListFiles(const char *directory, const char *extension, const char *filter, uint64_t *numfiles, qboolean wantsubs)
{
    struct dirent *d;
    DIR *fdir;
    qboolean dironly = wantsubs;
    char search[MAX_OSPATH*2+MAX_GDR_PATH+1];
    uint64_t nfiles;
    uint64_t extLen;
    uint64_t length;
    char **listCopy;
    char *list[MAX_FOUND_FILES];
    uint64_t i;
    struct stat st;
    qboolean hasPatterns;
    const char *x;

    if (filter) {
        nfiles = 0;
        Sys_ListFilteredFiles(directory, "", filter, list, &nfiles);

        list[nfiles] = NULL;
        *numfiles = nfiles;

        if (!nfiles)
            return NULL;
        
        listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC);
        for (i = 0; i < nfiles; i++) {
            listCopy[i] = list[i];
        }
        listCopy[i] = NULL;

        return listCopy;
    }

    if (!extension)
        extension = "";
    
    if (extension[0] == '/' && extension[1] == 0) {
        extension = "";
        dironly = qtrue;
    }

    if ((fdir = opendir(directory)) == NULL) {
        *numfiles = 0;
        return NULL;
    }

    extLen = strlen(extension);
    hasPatterns = Com_HasPatterns(extension);

    if (hasPatterns && extension[0] == '.' && extension[1] != '\0')
        extension++;
    
    // search
    nfiles = 0;

    while ((d = readdir(fdir)) != NULL) {
        if (nfiles >= MAX_FOUND_FILES - 1)
            break;

        // skip pwd '.' and '..', those'll cause a segfault in FS_PathCmp after FS_SortFileList
        if (d->d_name[0] == '.' || (d->d_name[0] == '.' && d->d_name[1] == '.')) {
            continue;
        }
        
        Com_snprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
        if (stat(search, &st) == -1)
            continue;
        
        if ((dironly && !(st.st_mode & S_IFDIR)) || (!dironly && (st.st_mode & S_IFDIR)))
            continue;
        
        if (*extension) {
            if (hasPatterns) {
                x = strrchr(d->d_name, '.');
                if (!x || !Com_FilterExt(extension, x + 1)) {
                    continue;
                }
            }
            else {
                length = strlen(d->d_name);
                if (length < extLen || N_stricmp(d->d_name + length - extLen, extension)) {
                    continue;
                }
            }
        }
        list[nfiles] = FS_CopyString(d->d_name);
        nfiles++;
    }

    list[nfiles] = NULL;

    closedir(fdir);

    // return a copy of the list
    *numfiles = nfiles;

    if (!nfiles)
        return NULL;
    
    listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC);
    for (i = 0; i < nfiles; i++) {
        listCopy[i] = list[i];
    }
    listCopy[i] = NULL;

    Com_SortFileList(listCopy, nfiles, extension[0] != '\0');

    return listCopy;
}

void Sys_FreeFileList( char **list )
{
    uint64_t i;

    if ( !list ) {
        return;
    }
    
    for ( i = 0; list[i]; i++ ) {
        Z_Free( list[i] );
    }
    Z_Free( list );
}

void Sys_GetRAMUsage( uint64_t *curVirt, uint64_t *curPhys, uint64_t *peakVirt, uint64_t *peakPhys )
{
    FILE *fp;
    char buf[1024];

    *curVirt = *peakVirt = 0;
    *curPhys = *peakPhys = 0;

    fp = fopen( "/proc/self/status", "r" );
    if ( !fp ) {
        return;
    }

    while ( fscanf( fp, " %1023s", buf ) == 1 ) {
        if ( strstr( buf, "VmRSS:" ) ) {
            fscanf( fp, " %lu", curPhys );
        }
        if ( strstr( buf, "VmHWM:" ) ) {
            fscanf( fp, " %lu", peakPhys );
        }
        if ( strstr( buf, "VmSize:" ) ) {
            fscanf( fp, " %lu", curVirt );
        }
        if ( strstr( buf, "VmPeak:" ) ) {
            fscanf( fp, " %lu", peakVirt );
        }
    }

    fclose( fp );
}


#define rdtsc( x ) \
    __asm__ __volatile__ ( "rdtsc" : "=A" ( x ) )

class CTimeVal
{
public:
    CTimeVal( void ) = default;
    CTimeVal& operator=( const CTimeVal& val ) { m_TimeVal = val.m_TimeVal; }
    inline double operator-( const CTimeVal& left ) {
        uint64_t left_us = (uint64_t)left.m_TimeVal.tv_sec * 1000000 + left.m_TimeVal.tv_usec;
	    uint64_t right_us = (uint64_t)m_TimeVal.tv_sec * 1000000 + m_TimeVal.tv_usec;
	    uint64_t diff_us = left_us - right_us;
	    return diff_us / 1000000.0f;
    }

    timeval m_TimeVal;
};

// Compute the positive difference between two 64 bit numbers.
static inline double diff( double v1, double v2 ) {
    double d = v1 - v2;
    return d >= 0 ? d : -d;
}


#ifdef OSX
double GetCPUFreqFromPROC( void )
{
    int mib[2] = { CTL_HW, HW_CPU_FREQ };
    uint64_t frequency = 0;
    size_t len = sizeof( frequency );

    if ( sysctl( mib, 2, &frequency, &len, NULL, 0 ) == -1 ) {
        return 0;
    }
    return (double)frequency;
}
#else
double GetCPUFreqFromPROC( void )
{
    double mhz = 0;
    char line[1024], *s, search_str[] = "cpu MHz";
    FILE *fp; 
    
    // open proc/cpuinfo
    if ( ( fp = fopen( "/proc/cpuinfo", "r" ) ) == NULL ) {
	    return 0;
    }

    // ignore all lines until we reach MHz information
    while ( fgets( line, 1024, fp ) != NULL ) { 
    	if ( strstr( line, search_str ) != NULL ) {
	        // ignore all characters in line up to :
	        for ( s = line; *s && ( *s != ':' ); ++s )
                ;
	        // get MHz number
	        if ( *s && ( sscanf( s+1, "%lf", &mhz ) == 1 ) ) {
		        break;
            }
	    }
    }

    if ( fp ) {
        fclose( fp );
    }

    return mhz * 1000000.0f;
}
#endif


double Sys_CalculateCPUFreq( void )
{
#ifdef __linux__
	const char *pFreq = getenv( "CPU_MHZ" );
	if ( pFreq ) {
		double retVal = 1000000.0f;
		return retVal * (double)atoll( pFreq );
	}
#endif

    // Compute the period. Loop until we get 3 consecutive periods that
    // are the same to within a small error. The error is chosen
    // to be +/- 0.02% on a P-200.
    const double error = 40000.0f;
    const int max_iterations = 600;
    int count;
    double period, period1 = error * 2, period2 = 0,  period3 = 0;

    for ( count = 0; count < max_iterations; count++ ) {
        CTimeVal start_time, end_time;
        uint64_t start_tsc, end_tsc;

        gettimeofday( &start_time.m_TimeVal, 0 );
        rdtsc( start_tsc );
        usleep( 5000 ); // sleep for 5 msec
        gettimeofday( &end_time.m_TimeVal, 0 );
        rdtsc( end_tsc );

        period3 = ( end_tsc - start_tsc) / (end_time - start_time );
        if ( diff( period1, period2 ) <= error && diff( period2, period3 ) <= error && diff( period1, period3 ) <= error ) {
 	        break;
        }
        period1 = period2;
        period2 = period3;
    }

    if ( count == max_iterations ) {
	    return GetCPUFreqFromPROC(); // fall back to /proc
    }

    // Set the period to the average period measured.
    period = ( period1 + period2 + period3 ) / 3;

    // Some Pentiums have broken TSCs that increment very
    // slowly or unevenly. 
    if ( period < 10000000.0f ) {
	    return GetCPUFreqFromPROC(); // fall back to /proc
    }

    return period;
}



/*
* Sys_GetFileStats: returns qtrue if the file exists
*/
qboolean Sys_GetFileStats( fileStats_t *stats, const char *path )
{
    struct stat fdata;

    if ( stat( path, &fdata ) != -1 ) {
        stats->mtime = fdata.st_mtime;
        stats->ctime = fdata.st_ctime;
        stats->exists = qtrue;
        stats->size = fdata.st_size;

        return qtrue;
    }

    stats->mtime = 0;
    stats->ctime = 0;
    stats->exists = qfalse;
    stats->size = 0;

    return qfalse;
}


#ifdef USE_AFFINITY_MASK
/*
=================
Sys_GetAffinityMask
=================
*/
uint64_t Sys_GetAffinityMask( void )
{
	cpu_set_t cpu_set;

	if ( sched_getaffinity( getpid(), sizeof( cpu_set ), &cpu_set ) == 0 ) {
		uint64_t mask = 0;
		int cpu;
		for ( cpu = 0; cpu < sizeof( mask ) * 8; cpu++ ) {
			if ( CPU_ISSET( cpu, &cpu_set ) ) {
				mask |= (1ULL << cpu);
			}
		}
		return mask;
	} else {
		return 0;
	}
}

/*
=================
Sys_SetAffinityMask
=================
*/
qboolean Sys_SetAffinityMask( const uint64_t mask )
{
	cpu_set_t cpu_set;
	int cpu;

	CPU_ZERO( &cpu_set );
	for ( cpu = 0; cpu < sizeof( mask ) * 8; cpu++ ) {
		if ( mask & (1ULL << cpu) ) {
			CPU_SET( cpu, &cpu_set );
		}
	}

	if ( sched_setaffinity( getpid(), sizeof( cpu_set ), &cpu_set ) == 0 ) {
		return qtrue;
	} else {
		return qfalse;
	}
}
#endif // USE_AFFINITY_MASK

/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int: 
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038
   using unsigned long data type to work right with Sys_XTimeToSysTime */
static uint64_t sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
     0x7fffffff ms - ~24 days
   although timeval:tv_usec is an int, I'm not sure whether it is actually used as an unsigned int
     (which would affect the wrap period) */

// [glnomad] changed unsigned long to a uint64_t
uint64_t Sys_Milliseconds(void)
{
    struct timeval tp;
    uint64_t curtime;

    gettimeofday(&tp, NULL);

    if (!sys_timeBase) {
        sys_timeBase = tp.tv_sec;
        return tp.tv_sec / 1000;
    }

    curtime = (tp.tv_sec - sys_timeBase) * 1000 + tp.tv_usec / 1000;
    return curtime;
}

uint64_t Sys_EventSubtime( uint64_t time )
{
    uint64_t ret, t, test;

	ret = com_frameTime - (uint64_t)( sys_timeBase * 1000 );
    t = Sys_Milliseconds();
    test = t - ret;

    if ( test < 0 || test > 30 ) {
        return t;
    }

    return ret;
}

uint64_t Sys_GetPageSize( void )
{
	return sysconf( _SC_PAGE_SIZE );
}

void *Sys_AllocVirtualMemory( size_t nBytes )
{
	void *pData = mmap( NULL, PAD( nBytes, Sys_GetPageSize() ), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
	if ( !pData || pData == MAP_FAILED ) {
		return NULL;
	}
	return pData;
}

void *Sys_CommitVirtualMemory( void *pMemory, size_t nBytes )
{
	const size_t size = PAD( nBytes, Sys_GetPageSize() );
	if ( mprotect( pMemory, size, PROT_WRITE | PROT_READ ) == -1 ) {
		return NULL;
	}
#ifdef MADV_WILLNEED
	madvise( pMemory, size, MADV_WILLNEED );
#elif defined(POSIX_MADV_WILLNEED)
	posix_madvise( pMemory, size, POSIX_MADV_WILLNEED );
#endif
	return pMemory;
}

void Sys_DecommitVirtualMemory( void *pMemory, size_t nBytes )
{
	const size_t size = PAD( nBytes, Sys_GetPageSize() );
#ifdef MADV_FREE
	madvise( pMemory, size, MADV_FREE );
#elif defined(MADV_DONTNEED)
	madvise( pMemory, size, MADV_DONTNEED );
#elif defined(POSIX_MADV_DONTNEED)
	posix_madvise( pMemory, size, POSIX_MADV_DONTNEED );
#endif
	if ( mprotect( pMemory, size, PROT_NONE ) == -1 ) {
		N_Error( ERR_FATAL, "Sys_DecommitVirtualMemory: mprotect PROT_NONE failed" );
	}
}

void Sys_ReleaseVirtualMemory( void *pMemory, size_t nBytes )
{
	if ( munmap( pMemory, PAD( nBytes, Sys_GetPageSize() ) ) == -1 ) {
		Con_Printf( COLOR_RED "ERROR: munmap failed on %lu!\n", nBytes );
	}
}

/*
* Sys_LockMemory: sets pAddress to ROM, if it wasn't in read permissions before,
* this'll probably fail
*/
void Sys_LockMemory( void *pAddress, uint64_t nBytes )
{
    if ( mprotect( pAddress, nBytes, PROT_READ ) == -1 ) {
        N_Error( ERR_FATAL, "Sys_LockMemory: mprotect failed!" );
    }
}

/*
* Sys_UnlockMemory: sets pAddress to read/write, if the memory wasn't write permitted before,
* this'll crash
*/
void Sys_UnlockMemory( void *pAddress, uint64_t nBytes )
{
    if ( mprotect( pAddress, nBytes, PROT_WRITE | PROT_READ ) == -1 ) {
        N_Error( ERR_FATAL, "Sys_UnlockMemory: mprotect failed!" );
    }
}

typedef struct {
    void *pAddress;
    size_t nBytes;
    int fileHandle;
    qboolean temporary;
} memoryMap_t;

static memoryMap_t *MapMemory( size_t nBytes, int prot, int flags, int fd )
{
    memoryMap_t *mem;

    mem = (memoryMap_t *)Z_Malloc( sizeof(*mem), TAG_STATIC );
    memset( mem, 0, sizeof(*mem) );
    mem->nBytes = nBytes;
    mem->temporary = !( flags & MAP_SHARED );
    mem->fileHandle = fd;

    mem->pAddress = mmap( NULL, nBytes, prot, flags, fd, 0 );
    if ( !mem->pAddress || mem->pAddress == MAP_FAILED ) {
        if ( fd != -1 ) {
            close( fd );
        }
        Z_Free( mem );

        N_Error( ERR_DROP, "Sys_CreateMemoryMap: failed to create memory map size %lu bytes, prot 0x%04x, flags 0x%04x, fd %i, strerror: %s",
            nBytes, prot, flags, fd, strerror( errno ) );
    }

    return mem;
}

void *Sys_MapFile( const char *path, qboolean temp )
{
    memoryMap_t *file;
    int prot, flags;
    int fd;
    size_t size;
    void *address;

    fd = open( path, 0 );
    if ( fd == -1 ) {
        N_Error( ERR_DROP, "Sys_MapFile: failed to create mapping for %s, strerror: %s", path, strerror( errno ) );
    }

    file = MapMemory( lseek( fd, 0, SEEK_END ), PROT_READ | PROT_WRITE, temp ? MAP_PRIVATE : MAP_SHARED, fd );
    lseek( fd, 0, SEEK_SET );

    return file;
}

void *Sys_VirtualAlloc( uint64_t size )
{
    return MapMemory( size, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1 );
}

void Sys_SetMemoryReadOnly( void *pAddress, uint64_t nBytes, qboolean isMapped )
{
    memoryMap_t *mem;

    if ( isMapped ) {
        mem = (memoryMap_t *)pAddress;

        if ( mprotect( mem->pAddress, nBytes, PROT_READ ) == -1 ) {
            N_Error( ERR_FATAL, "Sys_SetMemoryReadOnly: mprotect failed on memory mapped region, strerror: %s", strerror( errno ) );
        }
    }
    else {
        if ( mprotect( pAddress, nBytes, PROT_READ ) ) {
            N_Error( ERR_FATAL, "Sys_SetMemoryReadOnly: mprotect failed on memory region sized %lu bytes, strerror: %s", nBytes, strerror( errno ) );
        }
    }

    Con_DPrintf( "Set memory address %p -> %p (%lu bytes) to read only.\n", pAddress, (void *)( (byte *)pAddress + nBytes ), nBytes );
}

void *Sys_MapMemory( FILE *fp, qboolean temp, fileHandle_t fd )
{
    return NULL; // FIXME: implement
}

void *Sys_GetMappedFileBuffer( void *file )
{
    return NULL; // FIXME: implement
}

uint64_t Sys_ReadMappedFile( void *buffer, uint64_t size, void *file )
{
    return 0; // FIXME: implement
}

void Sys_UnmapMemory( void *pAddress, uint64_t nBytes )
{
}

void Sys_UnmapFile( void *file )
{
    memoryMap_t *mem;

    mem = (memoryMap_t *)file;

    if ( mem->fileHandle != -1 ) {
        close( mem->fileHandle );
    }
    if ( munmap( mem->pAddress, mem->nBytes ) == -1 ) {
        N_Error( ERR_FATAL, "Sys_UnmapMemory: munmap failed on %p, %lu bytes, strerror: %s", mem->pAddress, mem->nBytes, strerror( errno ) );
    }
    Z_Free( mem );
}

fileOffset_t Sys_TellMappedFile( void *file )
{
}

/*
* Sys_SeekMappedFile: use SEEK_SET, SEEK_END, and SEEK_CUR instead of custom filesystem seekers
*/
fileOffset_t Sys_SeekMappedFile( fileOffset_t offset, uint32_t whence, void *file )
{
}

#if 0
uint64_t Sys_GetCacheLine(void)
{
    uint64_t cacheline = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    sysconf(_SC_LEVEL2_CACHE_SIZE);
}
#endif

/*
Sys_GetTotalRAM_Physical: returns the total amount of physical RAM in the system
*/
uint64_t Sys_GetTotalRAM_Physical(void)
{
    uint64_t pageSize, pageCount;

    pageSize = (uint64_t)sysconf(_SC_PAGESIZE);
    pageCount = (uint64_t)sysconf(_SC_PHYS_PAGES);

    return pageSize * pageCount;
}

/*
Sys_GetTotalRAM_Virtual: returns the total amount of virtual RAM in the system
*/
uint64_t Sys_GetTotalRAM_Virtual(void)
{
    struct sysinfo info;

    if (sysinfo(&info) == -1) {
        N_Error(ERR_FATAL, "Sys_GetTotalRAM_Virtual: sysinfo() failed, error: %s", strerror(errno));
    }

    return info.totalram + info.totalswap * info.mem_unit;
}

/*
Sys_GetUsedRAM_Virtual: returns the total used virtual RAM in the system
*/
uint64_t Sys_GetUsedRAM_Virtual(void)
{
    struct sysinfo info;

    if (sysinfo(&info) == -1) {
        N_Error(ERR_FATAL, "Sys_GetFreeRAM_Virtual: sysinfo() failed, error: %s", strerror(errno));
    }

    return ((info.totalram - info.freeram) + info.totalswap - info.freeswap) * info.mem_unit;
}

/*
Sys_GetUsedRAM_Physical: returns the total used physical RAM in the system
*/
uint64_t Sys_GetUsedRAM_Physical(void)
{
    struct sysinfo info;

    if (sysinfo(&info) == -1) {
        N_Error(ERR_FATAL, "Sys_GetUsedRAM_Physical: sysinfo() failed, error: %s", strerror(errno));
    }

    return (info.totalram - info.freeram) * info.mem_unit;
}

qboolean Sys_mkdir(const char *name)
{
    if (mkdir(name, 0750) == 0) {
        return qtrue;
    }
    else {
        return (qboolean)(errno == EEXIST);
    }
}


int dll_err_count = 0;

/*
Sys_LoadDLL: all paths given to this are assumed to be absolute paths that won't be modified
*/
void *Sys_LoadDLL(const char *name)
{
    void *libHandle;
    const char *ext;

    if (FS_AllowedExtension(name, qfalse, &ext)) {
        N_Error(ERR_FATAL, "Sys_LoadDLL: unable to load library with '%s' extension", ext);
    }

    libHandle = dlopen(name, RTLD_NOW);
    if (!libHandle) {
        dll_err_count++;
    }
    return libHandle;
}

int Sys_GetDLLErrorCount( void ) {
    return dll_err_count;
}

const char *Sys_GetDLLError( void ) {
    if (dll_err_count) {
        dll_err_count--;
        return (const char *)dlerror();
    }
    return "no error";
}

void Sys_ClearDLLError( void ) {
    if (dll_err_count) {
        Con_DPrintf( COLOR_YELLOW "WARNING: clearing dll_err_count, but there's errors, listing them:\n" );
        for (int i = 0; i < dll_err_count; i++) {
            Con_DPrintf( COLOR_YELLOW "dll_error[%i]: %s\n", i, dlerror() );
        }
    }
    dll_err_count = 0;
}

void *Sys_GetProcAddress(void *handle, const char *name)
{
    void *proc;
    const char *err;
    char buf[1024];
    uint64_t nlen;

    if (!handle || !name || *name == '\0')
        return NULL;
    
    dlerror(); // clear the old error state
    proc = dlsym(handle, name);
    err = dlerror();
    if (err) {
        nlen = strlen(name);
        if (nlen >= sizeof(buf))
            return NULL;
        
        buf[0] = '_';
        strcpy(buf+1, name);
        dlerror(); // clear the old error state
        proc = dlsym(handle, name);
    }
    if (!proc) {
        dll_err_count++;
    }

    return proc;
}

void Sys_CloseDLL(void *handle)
{
    if (handle)
        dlclose(handle);
}

FILE *Sys_FOpen(const char *filepath, const char *mode)
{
    Assert( filepath );
    Assert( mode );
    
    if ( !*filepath ) {
        N_Error( ERR_FATAL, "Sys_FOpen: empty filepath" );
    }
    
    return fopen(filepath, mode);
}

const char *Sys_pwd( void )
{
    static char pwd[MAX_OSPATH];

    if ( *pwd ) {
        return pwd;
    }

    // more reliable, linux-specific
    if ( readlink( "/proc/self/exe", pwd, sizeof(pwd) - 1 ) != -1 ) {
        pwd[sizeof(pwd) - 1] = '\0';
        dirname( pwd );
        return pwd;
    }
    
    if ( !getcwd( pwd, sizeof(pwd) ) ) {
        *pwd = '\0';
    }

    return pwd;
}

char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( ( cliptext = SDL_GetClipboardText() ) != NULL ) {
		if ( cliptext[0] != '\0' ) {
			size_t bufsize = strlen( cliptext ) + 1;

			data = (char *)Z_Malloc( bufsize, TAG_STATIC );
			N_strncpyz( data, cliptext, bufsize );

			// find first listed char and set to '\0'
			strtok( data, "\n\r\b" );
		}
		SDL_free( cliptext );
	}
	return data;
}

const char *Sys_GetCurrentUser( void )
{
    struct passwd *pw;

    if ( ( pw = getpwuid( getuid() ) ) == NULL ) {
        return "player";
    }

    return pw->pw_name;
}
