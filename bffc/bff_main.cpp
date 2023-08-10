#include "../bff_file/g_bff.h"

typedef struct
{
    const char *argv[2];
    const char *description;
    void (*func)(int index);
} cmdarg_t;

static const char **myargv;
static int myargc;

static void usage(const char *msg)
{
    Con_Printf("usage: %s", msg);
    exit(1);
}

static void help(int index);
static void write(int index);
static void decompile(int index);
static void test(int index);

static const cmdarg_t cmdargs[] = {
    {{"-h", "--help"},      "-h --help                                   display this message", help},
    {{"-w", "--write"},     "-w --write OUTPUT.bff ENTRIES.json          write archive from a json entries file into a bff file", write},
    {{"-d", "--decompile"}, "-d --decompile INPUT.bff                    display the contents of a bff file", decompile},
    {{"-t", "--test"},      "-t --test INPUT.bff                         test a bff's contents (corruption proofing)", test},
};

static const char **Cmd_Args(void);
static int Cmd_Exists(const char *big, const char *little = NULL);
static const char *Cmd_Argv(int index);
static int Cmd_Argc(void);

int main(int argc, char** argv)
{
    myargc = argc;
    myargv = (const char **)argv;
    if (argc < 2) {
        Con_Printf("usage: %s [options...][arguments...]", argv[0]);
        for (const auto& i : cmdargs)
            Con_Printf("      %s", i.description);
    }
    for (int index = 0; index < argc; index++) {
        for (const auto& i : cmdargs) {
            if (!strcmp(argv[index], i.argv[0]) || !strcmp(argv[index], i.argv[1]))
                i.func(index);
        }
    }
    return 0;
}

static void write(int index)
{
    if (Cmd_Argc() < 3) {
        usage("-w|--write OUTPUT.bff ENTRIES.json");
    }

    const char *out = Cmd_Argv(index + 1);
    const char *entries = Cmd_Argv(index + 2);

    int compression = Cmd_Exists("-C", "--compress");
    int compressionLib = COMPRESSION_NONE;
    if (compression != -1) {
        const char *compressArg = Cmd_Argv(compression + 1);
        if (!strcmp(compressArg, "zlib")) {
            compressionLib = COMPRESSION_ZLIB;
            Con_Printf("using zlib compression");
        }
        else if (!strcmp(compressArg, "bzip2")) {
            compressionLib = COMPRESSION_BZIP2;
            Con_Printf("using bzip2 compression");
        }
        else
            usage("-w|--write OUTPUT.bff ENTRIES.json -C|--compress zlib|bzip2");
    }

    Con_Printf("writing archive from entries file %s to output %s", entries, out);

    WriteBFF(out, entries, compressionLib);
}

static void decompile(int index)
{
    if (Cmd_Argc() < 2) {
        usage("-d|--decompile INPUT.bff");
    }

    Con_Printf("displaying contents of bff archive %s", Cmd_Argv(index + 1));
    DecompileBFF(Cmd_Argv(index + 1));
}

static void help(int index)
{
    Con_Printf("usage: %s [options...][arguments...]", Cmd_Argv(0));
    for (const auto& i : cmdargs)
        Con_Printf("      %s", i.description);
}

static void test(int index)
{

}

static const char **Cmd_Args(void)
{
    return myargv;
}

static const char *Cmd_Argv(int index)
{
    if (index >= (unsigned)myargc)
        return "";
    
    return myargv[index];
}

static int Cmd_Argc(void)
{
    return myargc;
}

static int Cmd_Exists(const char *big, const char *little)
{
    for (int i = 0; i < Cmd_Argc(); ++i) {
        if (!strcmp(Cmd_Argv(i), big))
            return i;
        else if ((little) && !strcmp(Cmd_Argv(i), little))
            return i;
    }
    return -1;
}