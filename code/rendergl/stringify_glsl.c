#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    FILE *ifp;
    FILE *ofp;
    char buffer[1024];

    if(argc < 3) {
        fprintf(stderr, "usage: %s <input glsl file> <output file>\n", argv[0]);
        return 1;
    }

    char *inFile = argv[1];
    char *outFile = argv[2];

    ifp = fopen(inFile, "r");
    if(!ifp)
        return 2;

    ofp = fopen(outFile, "a");
    if(!ofp)
        return 3;

    // Strip extension
    char *base = basename(inFile);
    *strrchr(base, '.') = '\0';

    fprintf(ofp, "\nconst char *fallbackShader_%s =\n", base);

    while(fgets(buffer, sizeof(buffer), ifp))
    {
        // Strip trailing whitespace from line
        char *end = buffer + strlen(buffer) - 1;
        while(end >= buffer && isspace(*end))
            end--;

        end[1] = '\0';

        // Write line enquoted, with a newline
        fprintf(ofp, "\"%s\\n\"\n", buffer);
    }

    fprintf(ofp, ";\n");

    fclose(ifp);
    fclose(ofp);

    return 0;
}