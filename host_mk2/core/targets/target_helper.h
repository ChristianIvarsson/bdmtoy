#ifndef __TARGETS_HELPER_H__
#define __TARGETS_HELPER_H__

#include "../requests.h"
#include "../requests_cpu32.h"

typedef struct {
    const uint32_t    regCmd;
    const enDataSz    regSize;
    const uint32_t    spacing;
    const char *const name;
} regSummary_t;

class bdmstuff;

class target_helper
    : public virtual requests {

protected:
    bdmstuff & core;
    bool printReglist( const regSummary_t *, size_t count );

public:
    explicit target_helper( bdmstuff & m )
        : requests(m), core(m) {}
    virtual ~target_helper() {}

    virtual bool backupRegisters()  { return false; }
    virtual bool restoreRegisters() { return false; }
    virtual bool printRegisters()   { return false; }
    virtual bool runPC( uint64_t )  { return false; }
    virtual bool setPC( uint64_t )  { return false; }
    virtual bool getPC( uint64_t )  { return false; }
};

class helper_CPU32
    : public requests_cpu32,
      public target_helper {
public:
    explicit helper_CPU32( bdmstuff & m )
        : requests( m ), requests_cpu32( m ), target_helper( m ) { }
    ~helper_CPU32() {}

    bool printRegisters();
    bool runPC( uint64_t );
};

#endif
