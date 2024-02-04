#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

size_t openRead( const char *fName, FILE **fp ) {

    if ( (*fp = fopen( fName, "rb" )) == nullptr ) {
        printf("Error: openRead() - Could not get file handle\n");
        return 0;
    }

    fseek(*fp, 0L, SEEK_END);
    long fileSize = ftell(*fp);
    rewind(*fp);

    if ( fileSize < 0 ) {
        printf("Error: openRead() - Unable to determine file size\n");
        fclose( *fp );
        return 0;
    }

    if ( fileSize == 0 ) {
        printf("Error: openRead() - There's no data to read\n");
        fclose( *fp );
        return 0;
    }

    return (size_t)fileSize;
}

bool openWrite( const char *fName, FILE **fp ) {

    if ( (*fp = fopen( fName, "wb" )) == nullptr ) {
        printf("Error: openWrite() - Could not get file handle\n");
        return false;
    }

    rewind( *fp );
    return true;
}

bool writeLine( const char *buf, FILE *write ) {
    size_t length = strlen( buf );
    if ( length > 0) {
        if ( fwrite( buf, 1, length, write ) != length ) {
            printf("Error: writeLine() - Unable to write line\n");
            return false;
        }
        return true;
    }
    printf("Error: writeLine() - Tried to write a line of length 0\n");
    return false;
}

void makeHeader( FILE *read, size_t length, FILE *write, const char *hdr ) {
    char    txtBuf[ 256 ];
    uint8_t datBuf[ 16 ];

    printf("Outputting file..\n");

    if ( snprintf(txtBuf, sizeof(txtBuf), "static const uint8_t %s[] = {\r\n", hdr) < 1 ) {
        printf("Error: makeHeader() - Unable to generate output\n");
        return;
    }

    if ( !writeLine( txtBuf, write ) )
        return;


    while ( length > 0 ) {

        size_t offset = 4, toRead = (length > 12) ? 12 : length;

        // Read data
        if ( fread( datBuf, 1, toRead, read ) != toRead ) {
            printf("Error: makeHeader() - Unable to read byte from input\n");
            return;
        }

        if ( snprintf(txtBuf, sizeof(txtBuf), "    ") < 1 ) {
            printf("Error: makeHeader() - Unable to generate output\n");
            return;
        }

        for ( size_t i = 0; i < toRead; i++ ) {
            // Last line
            // "0x--" 4 chars
            if ( i == (toRead-1) && toRead == length ) {
                if ( snprintf(&txtBuf[ offset ], sizeof(txtBuf) - offset, "0x%02X", datBuf[ i ]) < 1 ) {
                    printf("Error: makeHeader() - Unable to generate output\n");
                    return;
                }
                offset += 4;
            // Middle of a line or still more lines to go
            // "0x--, " 6 chars
            } else {
                if ( snprintf(&txtBuf[ offset ], sizeof(txtBuf) - offset, "0x%02X, ", datBuf[ i ]) < 1 ) {
                    printf("Error: makeHeader() - Unable to generate output\n");
                    return;
                }
                offset += 6;
            }
        }

        if ( snprintf(&txtBuf[offset], sizeof(txtBuf) - offset, "\r\n") < 1 ) {
            printf("Error: makeHeader() - Unable to generate output\n");
            return;
        }

        if ( !writeLine( txtBuf, write ) )
            return;

        length -= toRead;
    }

    if ( snprintf(txtBuf, sizeof(txtBuf), "};\r\n") < 1 ) {
        printf("Error: makeHeader() - Unable to generate output\n");
        return;
    }

    writeLine( txtBuf, write );
}

int main(int argc, char *argv[]) {

    FILE  *readFp  = nullptr;
    FILE  *writeFp = nullptr;
    size_t readLen;

    if ( argc != 4 || argv[3] == 0 ) {
        printf("headertool <source.bin> <target.h> <array_name>\n");
        return 1;
    }

    if ( (readLen = openRead( argv[1], &readFp )) == 0 )
        return 1;

    if ( openWrite( argv[2], &writeFp ) ==  false ) {
        fclose( readFp );
        return 1;
    }

    makeHeader( readFp, readLen, writeFp, argv[3] );

    fclose( readFp );
    fclose( writeFp );

    return 0;
}
