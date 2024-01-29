#ifndef __BDMSTUFF_H__
#define __BDMSTUFF_H__

#include <cstdio>
#include <cstdint>
#include <chrono>

#include "usb/usb.h"

#include "targets/targets.h"

#include "../../shared/cmddesc.h"
#include "../../shared/enums.h"

class bdmstuff;

class timeout {
    std::chrono::time_point<std::chrono::system_clock> m_time;
    uint32_t m_ms;
public:
    explicit timeout() {
        m_time = std::chrono::system_clock::now();
        m_ms = 0;
    }
    void set(const uint32_t msTimeout) {
        m_time = std::chrono::system_clock::now();
        m_ms = msTimeout;
    }
    void reset() {
        m_time = std::chrono::system_clock::now();
    }
    // true = timeout, false = time left
    bool outatime() {
        return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_time).count() >= m_ms);
    }
};

// bdmworker.cpp
class bdmworker
    : public usb {
    
    bdmstuff &core;

    // Command queue as presented to the outside world
    class queue {
        bdmworker &worker;

    friend bdmworker;

        volatile bool &inFlight; // Something has been sent and a response is to be expected
        uint16_t &txSize;        // Total size of tx request (in words)
        uint16_t &txCommands;    // Number of currently stored commands in the queue
        uint32_t  txIndex;       // Where from to start storing commands
        uint16_t  txBuffer [ ADAPTER_BUFzIN/2 ];

        // Special command for assisted flash
        bool oneShot();

    public:
        explicit queue( bdmworker & wrk )
            : worker       ( wrk                ),
              inFlight     ( wrk.flags.inFlight ),
              txSize       ( txBuffer[0]        ),
              txCommands   ( txBuffer[1]        ),
              sentCommands ( txBuffer[1]        ) {
            queue::reset();
        }

        // External needs to know how many commands were sent
        const uint16_t &sentCommands;

        // Clear queue
        void reset();

        // Start new queue
        void begin( const uint16_t * );

        // Add to queue
        void add( const uint16_t * );

        // Send queue, wait for answer
        bool send();
        // Send a single command, wait for answer. (will reset any queued commands!)
        bool send( const uint16_t * );

        // Wait for status and return it
        bool getResult();

        // Operator shortcuts
        void operator  = ( const uint16_t *cmd ) { queue::begin ( cmd ); }
        void operator += ( const uint16_t *cmd ) { queue::add   ( cmd ); }
    };

    void updateProgress();

    // Communication
    void setFlags     ( const uint16_t * );  // Adapter --> host      - Set flags
    void recData      ( const uint16_t * );  // Adapter --> host      - Adapter has sent data to the host
    void sndData      ( const uint16_t * );  // Adapter <-> host      - Adapter has requested data from the host
    void scanRxQueue  ( const uint16_t * );  // Adapter --> host      - Iterate over received commands/data and act accordingly

    timeout timer;

    struct {
        bool     autoProgress = true;   // Should the download / upload code update the progress bar automatically?
        int32_t  lastProgress = 0;      // Last known progress
        uint64_t startAt      = 0;      // Where to start from
        uint64_t expectAt     = 0;      // Expect chunk starting from this address
        uint64_t stopAt       = 0;      // Stop when this address has been reached
    } memory;

    struct {
        uint8_t *buffer    = nullptr;
        size_t   allocated = 0;
        size_t   location  = 0;
        size_t   readSize  = 0;
    } file;

    struct {
        bool     multiFrame  = false;         // Receiving in chunks 
        uint32_t expected    = 0;             // How many words to expect
        uint32_t location    = 0;             // Location inside buffer
        uint16_t buffer[ ADAPTER_BUFzOUT/2 ]; // Temporary storage for rx until the whole frame has been received
    } rxTemp;

    // Should be part of queue
    struct {
        uint32_t nStoredCommands; // Number of stored commands
        uint16_t buffer[ ADAPTER_BUFzOUT/2 ];
    } rxQueue;

    struct {
        volatile bool     inFlight    = false;
                 uint16_t lastFault   = RET_OK;
    } flags;

    // Reset number of stored commands
    void resetRxCount();

    // Reset counters in rxTemp
    void resetRxTmp();

public:
    explicit bdmworker( bdmstuff & );
    ~bdmworker();

    // -- usb:: --
    // bool connect     (                  )
    // bool disconnect  (                  )
    // bool send        ( void *, uint32_t )

    // libUSB needs a couple of static callbacks so this can't be protected or private...
    void receive( const void *, uint32_t );

    // Update status and return true if status == RET_OK
    // - sets "queue.inFlight = false", no further comms will happen after
    bool flagStatus( uint16_t );

    // void begin     ( * Request );  ( Also usable as operator '='  )
    // void add       ( * Request );  ( Also usable as operator '+=' )
    // bool oneShot   (           );
    // bool oneShot   ( * Request );
    // bool send      (           );
    // bool send      ( * Request );
    // bool getResult (           );
    queue queue;

    // Reset to a known state
    // This will:
    // - Restore timeout to "GLOBALTIMEOUT"
    // - Turn auto progress on
    // - Return file location to offset 0
    // - Reset multiFrame to false
    //
    // - Reset queue              ( queue.reset() )
    // - - - No stored commands
    // - - - Not waiting for response
    // - - - No flagged faults
    // - - - Received command count 0
    void reset();

    // How long to wait for an answer before it's considered a fail
    void setTimeout( uint32_t );

    // Should the worker update progress on its own   ( Devices with paged memory needs this off when reading/writing more than one page )
    void autoProgress( bool );

    // Prepare worker for a download or upload        ( It has to know when to stop )
    void setRange( const memory_t * );

    // Expected file size limit. It's already set to 8 megs by default but if you know you'll need more, you can just request a new size
    void setSize( size_t );

    // Return last recorded status                    ( Should be one of "enum ReturnCodes" but can sometimes be custom )
    uint16_t lastFault();

    // Retrieve data from rx buffer
    bool getData( uint16_t       *buf,
                  Master_Commands toCmd,       // Which type of command has stored data
                  uint32_t        size,        // How many BYTES to expect in that request
                  uint32_t        index = 0 ); // If not the first matching command, which index?

    // Retrieve target status
    bool getStatus(uint16_t *sts = nullptr);

    // Byteswap filebuffer where blockSize can be 1 - 16
    bool swapDump  ( uint32_t blockSize );
    bool swapBuffer( uint32_t blockSize, size_t nBytes );

    // Mirror file buffer - Only use this after reading a file
    bool mirrorReadFile( size_t toSize );

    bool saveFile(const char*);
    bool readFile(const char*);

    const size_t & fileSize;
};


// bdmstuff.cpp
class bdmstuff
    : public bdmworker {

          iTarget  *target = nullptr;
    const target_t *desc   = nullptr;

    void unload();

public:
    explicit bdmstuff() : bdmworker(*this) {}
    ~bdmstuff() { unload(); }

    static constexpr const char * const version    = "0.0.1";
    static constexpr const int32_t      numTargets = sizeof ( targets ) / sizeof( target_t * );

    virtual void castProgress(int32_t) {}
    virtual void castMessage(const char *,...) = 0;

    // Load target
    bool load( int32_t );

    // Get name of target
    const char *name();
    static const char *name( int32_t idx ) {
        if ( idx < 0 || idx >= numTargets )
            return "Index out of bounds";
        if ( targets[ idx ]->name == nullptr || targets[ idx ]->name[0] == 0 )
            return "Not named";
        return targets[ idx ]->name;
    }

    // Get descriptor for target
    const target_t *info();
    static const target_t *info( int32_t idx ) {
        if ( idx < 0 || idx >= numTargets )
            return &errDesc;
        return targets[ idx ];
    }

    bool read();
    bool read( uint32_t );

    bool write();
    bool write( uint32_t );
};

class stopwatch {
    std::chrono::time_point<std::chrono::system_clock> m_start, m_now;
    uint32_t m_ms, m_sec, m_min, m_hr;
    time_t   m_msTot;
    void update() {
        m_msTot =  std::chrono::duration_cast<std::chrono::milliseconds>(m_now - m_start).count();
        m_ms    =  std::chrono::duration_cast<std::chrono::milliseconds>(m_now - m_start).count() % 1000;
        m_sec   = (std::chrono::duration_cast<std::chrono::milliseconds>(m_now - m_start).count() / 1000) % 60;
        m_min   = (std::chrono::duration_cast<std::chrono::milliseconds>(m_now - m_start).count() / (1000 * 60)) % 60;
        m_hr    = (std::chrono::duration_cast<std::chrono::milliseconds>(m_now - m_start).count() / (1000 * 60 * 60)) % 60;
    }
public:
    explicit stopwatch()
        : msTotal(m_msTot), milliseconds(m_ms), seconds(m_sec), minutes(m_min), hours(m_hr) {
        m_start = std::chrono::system_clock::now();
        m_now = m_start;
        update();
    }

    void capture() {
        m_start = m_now;
        m_now = std::chrono::system_clock::now();
        update();
    }

    const time_t   & msTotal;
    const uint32_t & milliseconds;
    const uint32_t & seconds;
    const uint32_t & minutes;
    const uint32_t & hours;
};

void sleep_ms(uint32_t waitms);

#endif
