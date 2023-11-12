#ifndef _SYS_UNIX_
#define _SYS_UNIX_

#pragma once

#include <sys/types.h>
#include <sys/errno.h>
#include <dlfcn.h>
#include <libgen.h>
#include <malloc.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <signal.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>

extern int dll_err_count;

#endif