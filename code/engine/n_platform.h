#ifndef __N_PLATFORM__
#define __N_PLATFORM__

#pragma once

#ifdef _WIN32
    #define IsPlatformLinux() 0
    #define IsPlatformPosix() 0
    #define IsPlatformOSX() 0
    #define IsPlatformPS5() 0
#elif defined(Q3_VM) || defined(__unix__)

#else
    #error "Wtf R U Compiling On?"
#endif

#endif
