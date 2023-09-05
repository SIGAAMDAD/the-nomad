#include "code/rendergl/rgl_public.h"
#include "code/engine/n_shared.h"
#include "sys_unix.h"
#include "code/engine/n_scf.h"
#include "code/engine/n_sound.h"
#include <execinfo.h>
#include <sys/sysinfo.h>

#define SYS_BACKTRACE_MAX 1024

GDR_INLINE void Sys_DoBacktrace(int amount)
{
    if (!FS_Initialized()) {
        return;
    }
    if (amount > SYS_BACKTRACE_MAX) {
        Con_Printf(WARNING, "Attempted to stacktrace > %i, aborting", SYS_BACKTRACE_MAX);
        return;
    }

    void **arr;
    char *buffer;
    FILE *tempfp;
    const char *ospath;
    int size;
    uint64_t fileLength;
    {
        arr = (void **)alloca(sizeof(void *) * amount);
        size = backtrace(arr, amount);
        ospath = FS_BuildOSPath(Cvar_VariableString("fs_basepath"), NULL, "backtrace.dat");
        tempfp = Sys_FOpen(ospath, "w+");
        if (!tempfp) {
            FS_Printf(logfile, C_RED "ERROR:" C_RESET " Failed to open a backtrace file");
            fprintf(stderr, C_RED "ERROR:" C_RESET " Failed to open a backtrace file\n");
            Con_Shutdown();
            Com_Shutdown();
            exit(-1);
        }

        // write the backtrace
        backtrace_symbols_fd(arr, size, fileno(tempfp));
    }

    fseek(tempfp, 0L, SEEK_END);
    fileLength = ftell(tempfp);
    fseek(tempfp, 0L, SEEK_SET);

    buffer = (char *)alloca(fileLength);
    fread(buffer, fileLength, 1, tempfp);

    Con_Printf("Successfully obtained %i stack frames", size);
    Con_Printf("Stack List:\n%s", buffer);

    fclose(tempfp);
}

typedef struct
{
    uint32_t id;
    qboolean safe;
    const char *str;
} exittype_t;

static const exittype_t signals[] = {
    {SIGSEGV,  qfalse, "segmentation violation"},
    {SIGBUS,   qfalse, "bus error"},
    {SIGFPE,   qfalse, "floating point exception"},
    {SIGABRT,  qfalse, "abnormal program termination"},
    {SIGSTOP,  qtrue,  "pausing program"},
    {SIGTERM,  qtrue,  "program termination"},
    {SIGILL,   qtrue,  "illegal instruction"},
    {SIGTRAP,  qtrue,  "debug breakpoint"},
    {0,        qtrue,  "No System Error"}
};

static const exittype_t *exit_type;

void GDR_NORETURN Sys_Exit(int code)
{
    const char *err;
#ifdef _NOMAD_DEBUG
    const bool debug = true;
#else
    const bool debug = false;
#endif

    // we're in developer mode and/or debug mode, print a stacktrace
    if ((Cvar_VariableInteger("c_devmode") || debug) && code == -1) {
        Sys_DoBacktrace(SYS_BACKTRACE_MAX);
    }

    if (code == -1) {
        if (exit_type)
            err = exit_type->str;
        else
            err = "No System Error";
        if (N_stricmp("No System Error", err) != 0)
            Con_Printf(ERROR, "Exiting With System Error: %s", err);
        else
            Con_Printf(ERROR, "Exiting With Engine Error");
    }
    
    Con_Shutdown();
    Com_Shutdown();

    if (code == -1)
        exit(EXIT_FAILURE);
    
    exit(EXIT_SUCCESS);
}

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
        snprintf(search, sizeof(search), "%s/%s", basedir, subdirs);
    else
        snprintf(search, sizeof(search), "%s", basedir);
    
    if ((fdir = opendir(search)) == NULL)
        return;
    
    while ((d = readdir(fdir)) != NULL) {
        snprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
        if (stat(filename, &st) == -1)
            continue;
        
        if (st.st_mode & S_IFDIR) {
            if (!N_streq(d->d_name, ".") && !N_streq(d->d_name, "..")) {
                if (*subdirs)
                    snprintf(newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
                else
                    snprintf(newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
                
                Sys_ListFilteredFiles(basedir, newsubdirs, filter, list, numfiles);
            }
        }
        if (*numfiles >= MAX_FOUND_FILES - 1)
            break;

        snprintf(filename, sizeof(filename), "%s/%s", subdirs, d->d_name);
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
        
        listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC, &listCopy, "fileList");
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
        
        snprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
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
    
    listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC, &listCopy, "fileList");
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
                N_Error("Unable to create directory \"%s\", error is %s(%d)", homePath, strerror(errno), errno);
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

	ret = Cvar_VariableInteger("com_frameTime") - (uint64_t)(sys_timeBase * 1000);
    t = Sys_Milliseconds();
    test = t - ret;

    if (test < 0 || test > 30) {
        return t;
    }

    return ret;
}

void Sys_Print(const char *str)
{
    const uint64_t len = strlen(str);
    if (write(STDOUT_FILENO, str, len) != len) {
        N_Error("Sys_Print: bad write");
    }
}

void GDR_DECL Sys_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[MAX_MSG_SIZE];
    int32_t length;

    va_start(argptr, fmt);
    length = stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Sys_Printf: overflow of %i bytes where buffer is %lu bytes", length, sizeof(msg));
    }

    write(STDOUT_FILENO, msg, length);
}

struct memoryMap_s
{
    void *addr;
    file_t file;
    int fd;
};

memoryMap_t *Sys_MapFile(const char *path, qboolean temp)
{
    memoryMap_t *file;
    
    file = (memoryMap_t *)Z_Malloc(sizeof(*file), TAG_STATIC, &file, "mappedFile");

    file->file = FS_FOpenRW(path);
    if (file->file == FS_INVALID_HANDLE) {
        Z_Free(file);
        return NULL;
    }

    file->fd = FS_FileToFileno(file->file);
    file->addr = mmap(NULL, FS_FileLength(file->file), PROT_READ | PROT_WRITE, temp ? MAP_PRIVATE : MAP_SHARED, file->fd, 0);
    if (!file->addr || file->addr == (void *)-1) { // mmap failed
        Z_Free(file);
        return NULL;
    }

    return file;
}

memoryMap_t *Sys_MapMemory(FILE *fp, qboolean temp, file_t fd)
{
    memoryMap_t *file;

    if (fd == FS_INVALID_HANDLE) {
        N_Error("Sys_MapMemory: invalid file handle given");
    }

    file = (memoryMap_t *)Z_Malloc(sizeof(*file), TAG_STATIC, &file, "mappedFile");

    file->file = fd;
    file->fd = fileno(fp);
    file->addr = mmap(NULL, FS_FileLength(fd), PROT_READ | PROT_WRITE, temp ? MAP_PRIVATE : MAP_SHARED, file->fd, 0);
    if (!file->addr || file->addr == (void *)-1) { // mmap failed
        Z_Free(file);
        return NULL;
    }

    return file;
}

void *Sys_GetMappedFileBuffer(memoryMap_t *file)
{
    return file->addr;
}

uint64_t Sys_ReadMappedFile(void *buffer, uint64_t size, memoryMap_t *file)
{
    int64_t readCount;
    uint64_t remaining;
    int tries;
    byte *buf;

    buf = (byte *)buffer;
    remaining = size;
    tries = 0;

    while (remaining) {
        readCount = read(file->fd, buf, remaining);
        if (readCount == 0) {
            if (!tries) {
                tries = 1;
            }
            else {
                return size - remaining;
            }
        }
        if (readCount == -1) {
            N_Error("Sys_ReadMappedFile: read -1 bytes");
        }

        buf += readCount;
        remaining -= readCount;
    }
    return size;
}

void Sys_UnmapMemory(memoryMap_t *file)
{
    munmap(file->addr, FS_FileLength(file->file));
}

void Sys_UnmapFile(memoryMap_t *file)
{
    munmap(file->addr, FS_FileLength(file->file));    
    FS_FClose(file->file);
    Z_Free(file);
}

fileOffset_t Sys_TellMappedFile(memoryMap_t *file)
{
    return (fileOffset_t)lseek(file->fd, 0, SEEK_CUR);
}

/*
Sys_SeekMappedFile: use SEEK_SET, SEEK_END, and SEEK_CUR instead of custom filesystem seekers
*/
fileOffset_t Sys_SeekMappedFile(fileOffset_t offset, uint32_t whence, memoryMap_t *file)
{
    return (fileOffset_t)lseek(file->fd, offset, whence);
}

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
        N_Error("Sys_GetTotalRAM_Virtual: sysinfo() failed, error: %s", strerror(errno));
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
        N_Error("Sys_GetFreeRAM_Virtual: sysinfo() failed, error: %s", strerror(errno));
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
        N_Error("Sys_GetUsedRAM_Physical: sysinfo() failed, error: %s", strerror(errno));
    }

    return (info.totalram - info.freeram) * info.mem_unit;
}

/*
Sys_LoadDLL: all paths given to this are assumed to be absolute paths that won't be modified
*/
void *Sys_LoadDLL(const char *name)
{
    void *libHandle;
    char ospath[MAX_OSPATH*2];

    // allow a little bit of pedanticity
    GDR_ASSERT(name);

    if (name[0] == '/')
        snprintf(ospath, sizeof(ospath), ".%s" ARCH_STRING DLL_EXT, name);
    else
        snprintf(ospath, sizeof(ospath), "./%s" ARCH_STRING DLL_EXT, name);

    libHandle = dlopen(ospath, RTLD_NOW);
    if (!libHandle) {
        Con_Printf(ERROR, "Sys_LoadDLL: failed, dlerror(): %s", dlerror());
    }
    return libHandle;
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
        N_Error("Sys_FOpen: no filepath given");
    if (!mode)
        N_Error("Sys_FOpen: no mode given");
    
    if (!*filepath)
        N_Error("Sys_FOpen: empty filepath");
    
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

void Catch_Signal(int signum)
{
    for (uint32_t i = 0; i < arraylen(signals); i++) {
        if (signals[i].id == signum)
            exit_type = &signals[i];
    }
    if (!exit_type)
        exit_type = &signals[arraylen(signals) - 1];
    
    Sys_Exit(-1);
}

int main(int argc, char **argv)
{
    Con_Printf("Working directory: %s", Sys_pwd());

    signal(SIGSEGV, Catch_Signal);
    signal(SIGBUS, Catch_Signal);
    signal(SIGFPE, Catch_Signal);
    signal(SIGABRT, Catch_Signal);
    signal(SIGTERM, Catch_Signal);
    signal(SIGILL, Catch_Signal);
    signal(SIGSTOP, Catch_Signal);
    signal(SIGKILL, Catch_Signal);

    I_NomadInit();

    return 0;
}