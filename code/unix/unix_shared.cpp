#include "../engine/n_shared.h"
#include "sys_unix.h"

qboolean Sys_RandomBytes(byte *s, uint64_t len)
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

//
// Sys_StackMemoryRemaining: returns the amount of stack we have left
//
uint64_t Sys_StackMemoryRemaining(void)
{
    struct rlimit limit;
    
    getrlimit(RLIMIT_STACK, &limit);

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
            if (!N_streq(d->d_name, ".") && !N_streq(d->d_name, "..")) {
                if (*subdirs)
                    Com_snprintf(newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
                else
                    Com_snprintf(newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
                
                Sys_ListFilteredFiles(basedir, newsubdirs, filter, list, numfiles);
            }
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
        
        Com_snprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
        if (stat(search, &st) == -1)
            continue;
        
        if ((dironly && !(st.st_mode & S_IFDIR)) || (!dironly && (st.st_mode & S_IFDIR)))
            continue;
        
        if (d->d_name[0] == '.' || (d->d_name[0] == '.' && d->d_name[1] == '.')) {
            continue;
        }
        
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

void Sys_FreeFileList(char **list)
{
    if (!list)
        return;
    
    for (uint64_t i = 0; list[i]; i++) {
        Z_Free(list[i]);
    }
    Z_Free(list);
}

const char *Sys_DefaultBasePath(void)
{
    return Sys_pwd();
}

const char *Sys_DefaultHomePath(void)
{
    // used to determine where to store user-specific files
    static char homePath[MAX_OSPATH];

    const char *p;

    if (*homePath)
        return homePath;
    
    if ((p = getenv("HOME")) != NULL) {
        N_strncpyz(homePath, p, sizeof(homePath));
#ifdef MACOS_X
        N_strcat(homePath, sizeof(homePath), "/Library/Application Support/TheNomad");
#else
        N_strcat(homePath, sizeof(homePath), "/.thenomad");
#endif
        if (mkdir(homePath, 0750)) {
            if (errno != EEXIST)
                N_Error(ERR_DROP, "Unable to create directory \"%s\", error is %s(%d)", homePath, strerror(errno), errno);
        }
        return homePath;
    }
    return ""; // assume current directory
}

/*
Sys_GetFileStats: returns qtrue if the file exists
*/
qboolean Sys_GetFileStats(fileStats_t *stats, const char *path)
{
    struct stat fdata;

    if (stat(path, &fdata) != -1) {
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

uint64_t Sys_EventSubtime(uint64_t time)
{
    uint64_t ret, t, test;

	ret = com_frameTime - (uint64_t)(sys_timeBase * 1000);
    t = Sys_Milliseconds();
    test = t - ret;

    if (test < 0 || test > 30) {
        return t;
    }

    return ret;
}

typedef struct {
    file_t file;
    int fd;
} mappedFile_t;

struct memoryMap_s
{
    void *addr;
    union {
        mappedFile_t file;
        uint64_t size;
    };
};

memoryMap_t *Sys_MapFile(const char *path, qboolean temp)
{
    return NULL; // FIXME: implement
}

memoryMap_t *Sys_VirtualAlloc(uint64_t size)
{
    return NULL; // FIXME: implement
}


memoryMap_t *Sys_MapMemory(FILE *fp, qboolean temp, file_t fd)
{
    return NULL; // FIXME: implement
}

void *Sys_GetMappedFileBuffer(memoryMap_t *file)
{
    return NULL; // FIXME: implement
}

uint64_t Sys_ReadMappedFile(void *buffer, uint64_t size, memoryMap_t *file)
{
    return 0; // FIXME: implement
}

void Sys_UnmapMemory(memoryMap_t *file)
{
}

void Sys_UnmapFile(memoryMap_t *file)
{
}

fileOffset_t Sys_TellMappedFile(memoryMap_t *file)
{
}

/*
Sys_SeekMappedFile: use SEEK_SET, SEEK_END, and SEEK_CUR instead of custom filesystem seekers
*/
fileOffset_t Sys_SeekMappedFile(fileOffset_t offset, uint32_t whence, memoryMap_t *file)
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
    if (!filepath)
        N_Error(ERR_FATAL, "Sys_FOpen: no filepath given");
    if (!mode)
        N_Error(ERR_FATAL, "Sys_FOpen: no mode given");
    
    if (!*filepath)
        N_Error(ERR_FATAL, "Sys_FOpen: empty filepath");
    
    return fopen(filepath, mode);
}

const char *Sys_pwd(void)
{
    static char pwd[MAX_OSPATH];

    if (*pwd)
        return pwd;

    // more reliable, linux-specific
    if (readlink("/proc/self/exe", pwd, sizeof(pwd) - 1) != -1) {
        pwd[sizeof(pwd) - 1] = '\0';
        dirname(pwd);
        return pwd;
    }
    
    if (!getcwd(pwd, sizeof(pwd))) {
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
