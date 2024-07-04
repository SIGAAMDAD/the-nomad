#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <libgen.h>
#include <filesystem>

int main( int argc, char **argv )
{
    FILE *ifp;
    FILE *ofp;
    char *bufp;
    char buffer[1024];

    if ( argc < 3 ) {
        fprintf( stdout, "%s <input directory> <output file>\n", argv[0] );
        return 1;
    }

    char *inDirectory = argv[1];
    char *outFile = argv[2];

    ofp = fopen( outFile, "w" );
    if ( !ofp ) {
        return 2;
    }

    for ( const auto& it : std::filesystem::directory_iterator{ argv[1] } ) {
        std::string path = it.path().string();

        ifp = fopen( path.c_str(), "r" );
        if ( !ifp ) {
            return 3;
        }

        // Strip extension
        char *base = basename( path.data() );
        *strrchr( base, '.' ) = '\0';

        fprintf( ofp, "\nconst char *fallbackShader_%s =\n", base );

        while ( fgets( buffer, sizeof( buffer ) - 1, ifp ) ) {
            // Strip trailing whitespace from line
            char *end = buffer + strlen( buffer ) - 1;
            while ( end >= buffer && isspace( *end ) ) {
                end--;
            }
            end[1] = '\0';

            fputc( '\"', ofp );

            bufp = buffer;
            while ( *bufp ) {
                switch ( *bufp ) {
                case '\"':
                    fprintf( ofp, "\\\"" );
                    break;
                case '\\':
                    fprintf( ofp, "\\\\" );
                    break;
                case '\'':
                    fprintf( ofp, "\\\'" );
                    break;
                default:
                    fputc( *bufp, ofp );
                    break;
                };
                bufp++;
            }
            fprintf( ofp, "\\n\"\n" );
        }
        fprintf( ofp, ";\n" );
        fclose( ifp );
    }
    fclose( ofp );

    return 0;
}
