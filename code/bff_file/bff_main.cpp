#include "g_bff.h"

#ifndef _NOMAD_VERSION

int main(int argc, char** argv)
{
    if (argc < 2) {
        Con_Printf(
            "usage: %s [options...][arguments...]\n"
            "   -c --compile OUTPUT.bff ENTRIES.json        compile data from a json entries file into a bff file\n"
            "   -d --decompile INPUT.bff                    display the contents of a bff file\n",
        argv[0]);
    }
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--compile")) {
            Con_Printf("compiling from entries file %s to output %s", argv[i+2], argv[i+1]);
            WriteBFF(argv[i+1], argv[i+2]);
        }
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--decompile")) {
            Con_Printf("displaying contents of bff archive %s", argv[i+1]);
            DecompileBFF(argv[i+1]);
        }
    }
    return 0;
}

#endif