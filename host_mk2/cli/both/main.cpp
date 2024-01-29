
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

#include "../../core/bdmstuff.h"

// While testing
#include <unistd.h>

class m_stuff : public bdmstuff {

public:
    void castProgress(int32_t) {

    }

    void castMessage(const char *str,...) {
        char tmp[256];
        va_list ap;
        va_start(ap, str);
        if (vsprintf(tmp, str, ap) > 0)
            printf("%s\n", tmp);
        va_end(ap);
    }


    ~m_stuff() {
        // Call before destruction of base. We want log messages!
        // disconnect();

        castMessage("m_stuff::~m_stuff()");
    }
};

m_stuff stuff;


/*
class testA {
public:
    explicit testA() {
        printf("TestA\n");
    }

    void test() {
        printf("testA:test()\n");
    }
};

class testB : public virtual testA {
public:
    explicit testB( testA & pr )
        : testA(pr) {
        printf("TestB\n");
    }
    void test() {
        printf("testB:test()\n");
    }
};
*/


int main(int argc, char *argv[]) {
    printf("\nAlive!\n");

/*
    testA tA;
    testB tB( tA );
*/

    printf("Test: %s\n", bdmstuff::name( 3 ));

    stuff.castMessage("Version: %s", stuff.version);
    // uint8_t buf[ 8 ] = { 0 };

    // stuff.connect();
    // stuff.send(buf,8);

    stuff.load( 3 );

    if ( stuff.readFile("flash.bin") ) {
        stuff.write( 0 );
    }

    // Read 
    if ( stuff.read( 0 ) ) {
        stuff.saveFile("dump.bin");
    }

    return 1;
}