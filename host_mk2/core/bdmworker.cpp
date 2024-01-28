//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Communication worker
//
// Look for <Mend me>!
// - - There's plenty of stuff that needs cleaning up

#include <cstdlib>

#include "bdmstuff.h"

bdmworker::bdmworker( bdmstuff & par )
    : usb( par, *this ),
      core( par ),
      queue( *this )
{
    if ( (file.buffer = (uint8_t*)malloc( 8 * 1024 * 1024 )) == nullptr ) {
        printf("Catastrophic error - Could not allocate file buffer\n");
        exit( EXIT_FAILURE );
    }

    file.length = 8 * 1024 * 1024;

    this->reset();
}

bdmworker::~bdmworker() {
    if ( file.buffer != nullptr )
        free( file.buffer );
    printf("bdmworker::~bdmworker() \n");
}

bool bdmworker::flagStatus(uint16_t flag) {
    flags.inFlight = false;
    return ((flags.lastFault = flag) == RET_OK);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal; Comms

// <Mend me>
// - FLAG is HIGHLY target-dependant and sadly also operation-dependant. Old code never bothered to read it so it's possible to clean that junk

// Adapter wants to update host flags
void bdmworker::setFlags( const uint16_t *rx ) {

    // [ LEN 4 ] [ SET_FLAG ] [ <STATUS> ] [ <FLAG> ]
    if ( rx[0] != 4 ) {
        core.castMessage("Error: setFlags() incorrect length");
        flagStatus( RET_MALFORMED );
        return;
    }

    core.castMessage((rx[2] == RET_OK) ? "Info: Driver done" : "Error: Adapter flagged a fault");
    core.castMessage("Flag: %04X", rx[3]);
}

void bdmworker::resetRxCount() {
    rxQueue.nStoredCommands = 0;
}

void ::bdmworker::resetRxTmp() {
    // Should be in queue
    rxTemp.multiFrame = false;
    rxTemp.expected = 0;
    rxTemp.location = 0;
}

// <Mend me>
// Update progress bar
void bdmworker::updateProgress() {
    if ( memory.autoProgress ) {
        int32_t percentage = (int32_t)(float)100.0 * (memory.expectAt - memory.startAt) / (memory.stopAt - memory.startAt);

        // Pushing progress is kinda slow. Only do it in 5% increments
        if ( percentage > (memory.lastProgress + 4) || percentage < memory.lastProgress ) {        
            if (percentage > 100)
                percentage = 100;
            core.castProgress( memory.lastProgress = percentage );
        }
    }
}

/// To adapter:
// Header    : [total len, words], [no. payloads]
// Payload(s): [[cmd], [cmd + data len, words], [data (if present)]]

/// From adapter:
// Regular commands
// [total len, words], Payload[[cmd], [ status  ], [cmd len], [data (if present)]] ..next payload

// Dump OK:
// [total len, words], Payload[[cmd], [    0    ], [addr][addr], [Data..]]
// Dump Fault
// [total len, words], Payload[[cmd], [faultcode]]

// [ 7 ] [ ASSIST_FLASH ] [ 0 ]
// <Mend me>
// Adapter wants data from the host
void bdmworker::sndData( const uint16_t *rx )
{

/*
        if ( temp[2] != RET_OK ) {
            flagStatus( temp[2] );
            return;
        }
        */
/*
    uint32_t  bufAddr  = *(uint32_t *) &rx[3];
    uint32_t  Len      = *(uint32_t *) &rx[5];
    uint16_t *flashPtr =  (uint16_t *) &file.buffer[ bufAddr / 2 ];

    // core.castMessage("Info: Adapter req'd data: %x, Len %u", bufAddr, Len);

    // Malformed request
    if ( rx[0] != 7 ) {
        core.castMessage("Error: sendFlashData() Incorrect length");
        flagStatus( RET_MALFORMED );
        return;
    }

    // Adapter tried to fetch more data than we have buffer!
    else if ( (bufAddr + Len) > file.length ) {
        core.castMessage("Error: sendFlashData() Bounds");
        flagStatus( RET_BOUNDS );
        return;
    }

    // McFly..?
    else if ( Len == 0 ) {
        core.castMessage("Error: sendFlashData() Malformed 2");
        flagStatus( RET_MALFORMED );
        return;
    }

    memory.expectAt = bufAddr + Len;

    updateProgress();

    // Update time since last. This'll make other functions wait indefinitely while flashing
    timer.reset();

    cmdQueue.txBuffer[0] = (uint16_t)((Len/2) + 3);
    cmdQueue.txBuffer[1] = 1;
    cmdQueue.txBuffer[2] = TAP_DO_ASSISTFLASH_IN;

    for (uint32_t i = 0; i < (Len/2); i++ )
        cmdQueue.txBuffer[3 + i] = *flashPtr++;

    resetRxReq();
    // resetTxReq();

    cmdQueue.queueMaster = TAP_DO_ASSISTFLASH_IN;

    sendRequest(cmdQueue.txBuffer);
*/
}

// <Mend me>
// [tot len in words (lenword inc)] [cmd][sts] [addr][addr] [Data..]
void bdmworker::recData( const uint16_t *rx )
{
    uint32_t  nBytes = (rx[0] - 5) * 2;

    // Adapter sent error message
    if ( rx[2] != RET_OK ) {
        core.castMessage("Error: recData() - Saw error message %04X", rx[2]);
        flagStatus( rx[2] );
        return;
    }

    // Adapter sent request but no data
    if ( rx[0] < 6 ) {
        core.castMessage("Error: recData() - no data ( %u (%08X : %08X) )", rx[0], *(uint32_t *) &rx[3], memory.expectAt);
        flagStatus( RET_MALFORMED );
        return;
    }
    
    // Adapter request to store data at the wrong location
    if ( *(uint32_t *) &rx[3] != memory.expectAt ) {
        core.castMessage("Error: recData() - wrong address ( %u (%08X : %08X) )", rx[0], *(uint32_t *) &rx[3], memory.expectAt);
        flagStatus( RET_FRAMEDROP );
        return;
    }

    // Check if there's enough room
    if ( (file.location + nBytes) > file.length ) {
        core.castMessage("Error: Adapter tried to send more data than the file buffer can hold");
        flagStatus( RET_BOUNDS );
        return;
    }

    // Sent too much data
    if ( (memory.expectAt + nBytes) > memory.stopAt ) {
        core.castMessage("Error: Adapter sent more data than requested");
        flagStatus( RET_BOUNDS );
        return;
    }

    // Update time since last. This'll make other functions wait indefinitely while dumping
    timer.reset();

    memcpy( &file.buffer[ file.location ], &rx[5], nBytes );

    memory.expectAt += nBytes;
    file.location   += nBytes;

    updateProgress();

    if ( memory.expectAt == memory.stopAt )
        flagStatus( RET_OK );
}

/// From adapter:
// Regular commands
// [total len, words], Payload[[cmd], [ status  ], [cmd len], [data (if present)]] ..next payload

// <Mend me>
// [tot len in words]  ([in response to req] [status] [len of resp in words(this word included)] [data if available])  ..
void bdmworker::scanRxQueue(const uint16_t *rx)
{
    uint16_t *strptr = rxQueue.buffer;
    uint32_t  totLen = *rx++;
    uint32_t  noRec  = 0;
    uint32_t  noData = 0;

    // Should never receive a frame smaller than 4 words
    if ( totLen < 4 ) {
        core.castMessage("Error: scanRxQueue() totLen < 4");
        flagStatus( RET_MALFORMED );
        return;
    }
    
    // Discard length-word from total length
    totLen--;

    do {
        uint16_t req    = rx[0];
        uint16_t status = rx[1];
        uint16_t reqLen = rx[2];

        // Check if response is too small
        if ( reqLen < 3 ) {
            core.castMessage("Error: scanRxQueue() reqLen < 3");
            flagStatus( RET_MALFORMED );
            return;            
        }

        // Check if request length goes outside of total length
        if ( reqLen > totLen ) {
            core.castMessage("Error: scanRxQueue() reqLen > totLen");
            flagStatus( RET_MALFORMED );
            return;            
        }

        if ( status != RET_OK ) {
            if ( req == TAP_DO_UPDATESTATUS ) {
                // Has its own message
                setFlags( rx );
                // Small caveat here. setFlags performs a flagStatus( RET_MALFORMED ) if length is not 4.
                if ( reqLen == 4 )
                    flagStatus( status );
            } else {
                core.castMessage("Error: Command %04X, failed with: %04X", req, status);
                flagStatus( status );
            }
            return;                
        }

        //  ([in response to req] [status] [len of resp in words(this word included)] [data if available])
        switch ( req ) {

            // Stored received queue format: [tot len]  ( [req][req len(this included)][data]+ )  ..
            case TAP_DO_TARGETSTATUS:
            case TAP_DO_READMEMORY:
            case TAP_DO_READREGISTER:
            case TAP_DO_ExecuteIns:

                // We expect these to contain some sort of data and not only a header
                if ( reqLen < 4 ) {
                    // TAP_DO_ExecuteIns is quite broad and won't return data in some use-cases
                    // Catch it and just ignore for now.
                    // - Fix this once the command is needed!
/*
                    if ( req == TAP_DO_ExecuteIns ) {
                        core.castMessage("wrk_scanQueue(): debug, no data");
                        // return;
                    }
*/
                    flagStatus( RET_MALFORMED );
                    return;       
                }

                // Store command
                *strptr++ = req;
                
                // Store length minus status-word (commands are only stored if the request went ok)
                *strptr++ = (reqLen - 1);
                
                // Store data
                for (uint32_t i = 0; i < (uint32_t)(reqLen - 3); i++)
                    *strptr++ = rx[3 + i];

                // Increment number of stored commands
                noData++;
                break;

            case TAP_DO_UPDATESTATUS:
                setFlags( rx );
                break;

            default:
                // core.castMessage("Info: Command without data %04X", req);    
                break;
        }

        totLen -= reqLen;
        rx     += reqLen;
        noRec++;

    } while ( totLen > 0 );

    if ( noRec != queue.sentCommands ) {
        core.castMessage("Error: Req missmatch! s(%u) r(%u)", queue.sentCommands, noRec);
        flagStatus( RET_MALFORMED );
        return;     
    }

    // Indicate number of stored commands in the Rx buffer
    rxQueue.nStoredCommands = noData;

    flagStatus( RET_OK );
}

// <Mend me>
// Since we might receive the frame in several chunks, we first have to buffer those and then forward the whole thing to their respective owners
void bdmworker::receive( const void *buf, uint32_t len ) {

    // core.castMessage("Info: receive() %u", recLen);;

    // We count in words, not bytes.
    len /= 2;

    if ( len == 0 )
        return;

    // Not expecting anything!
    if ( flags.inFlight == false )
        return;


    // Begin
    if ( rxTemp.multiFrame == false ) {
        rxTemp.location = 0;
        rxTemp.expected = *(uint16_t*)buf;

        if ( rxTemp.expected > (ADAPTER_BUFzOUT/2) ) {
            core.castMessage("Error: receive() Total count more than buffer can hold");
            flagStatus( RET_MALFORMED );
            return;
        }

        // Not seeing all the data that is required for this response. Expect more frames
        if ( rxTemp.expected > len )
            rxTemp.multiFrame = true;
    }

    // Adapter sent more data than header described.
    if ( len > rxTemp.expected ) {
        core.castMessage("Error: receive() received more than expected");
        flagStatus( RET_MALFORMED );
        return;
    }

    memcpy( &rxTemp.buffer[ rxTemp.location ], buf, len * 2 );

    rxTemp.location += len;
    rxTemp.expected -= len;

    if ( rxTemp.expected == 0 )
    {
        uint16_t *temp = rxTemp.buffer;

        rxTemp.multiFrame = false;

        switch ( temp[1] ) {
            // These two have a special format to maximize buffer usage
            case TAP_DO_DUMPMEM:
                recData( temp );
                return;

            case TAP_DO_ASSISTFLASH_IN:
                core.castMessage("Error: receive() TAP_DO_ASSISTFLASH_IN");
                flagStatus( RET_MALFORMED );
                // sndData( temp );
                return;

            default:
                scanRxQueue( temp );
                break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External; Requests - queue

/// To adapter:
// Header    : [total len, words], [no. payloads]
// Payload(s): [[cmd], [cmd + data len, words], [data (if present)]]

/// From adapter:
// Regular commands
// [total len, words], Payload[[cmd], [ status  ], [cmd len], [data (if present)]] ..next payload

// Dump OK:
// [total len, words], Payload[[cmd], [    0    ], [addr][addr], [Data..]]
// Dump Fault
// [total len, words], Payload[[cmd], [faultcode]]

void bdmworker::queue::reset() {
    txSize     = 2; // Header takes 2 words
    txCommands = 0; // No commands in queue
    txIndex    = 2; // Start adding commands from index 2 and up

    // - inFlight  = false;
    // - lastFault = RET_OK;
    worker.flagStatus( RET_OK );

    // Reset number of stored commands
    worker.resetRxCount();
}

bool bdmworker::queue::getResult() {

    // auto tick = std::chrono::system_clock::now();

    while ( inFlight && worker.timer.outatime() == false ) { }

    if ( inFlight && worker.timer.outatime() ) {
        inFlight = false;
        worker.core.castMessage("Error: queue::send() - Timeout!");
        sleep_ms( 500 ); // Wait 500 ms in an attempt to prevent a race condition
        worker.resetRxTmp();
        // worker.core.castMessage("Info: queue::getResult() spent %d ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tick).count());
        return worker.flagStatus( RET_TIMEOUT );
    }

    // worker.core.castMessage("Info: queue::getResult() spent %d ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tick).count());
    return (inFlight == false && worker.flags.lastFault == RET_OK);
}

// Start new queue
void bdmworker::queue::begin(const uint16_t *cmd) {
    queue::reset();
    queue::add( cmd );
}

// Add to queue
void bdmworker::queue::add(const uint16_t *cmd) {
    if ( cmd == nullptr ) {
        worker.core.castMessage("Debug: queue::add() - nullptr");
        return;
    }
    memcpy( &txBuffer[ txIndex ], cmd, cmd[ 1 ] * 2 );
    txSize  += cmd[ 1 ];
    txIndex += cmd[ 1 ];
    txCommands++;
}

// Send a single command, wait for response
bool bdmworker::queue::send(const uint16_t *cmd) {
    queue::begin( cmd );
    return queue::send();
}

// Send a single command, do NOT wait for response
bool bdmworker::queue::oneShot(const uint16_t *cmd) {
    queue::begin( cmd );
    return queue::oneShot();
}

// Send queue, wait for response
bool bdmworker::queue::send() {

    if ( txCommands == 0 ) {
        worker.core.castMessage("Error: queue::send() - No commands in queue");
        return worker.flagStatus( RET_MALFORMED );
    }

    inFlight = true;

    if ( worker.usb::send( (void*)txBuffer, txBuffer[ 0 ] * 2 ) == false ) {
        inFlight = false;
        sleep_ms( 500 );
        worker.resetRxTmp();
        return worker.flagStatus( RET_USBERR );
    }

    worker.timer.reset();

    return getResult();
}

// Send queue, do NOT wait for response
bool bdmworker::queue::oneShot() {

    if ( txCommands == 0 ) {
        worker.core.castMessage("Error: queue::oneShot() - No commands in queue");
        return worker.flagStatus( RET_MALFORMED );
    }

    inFlight = true;

    if ( worker.usb::send( (void*)txBuffer, txBuffer[ 0 ] * 2 ) == false ) {
        inFlight = false;
        sleep_ms( 500 );
        worker.resetRxTmp();
        return worker.flagStatus( RET_USBERR );
    }

    worker.timer.reset();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External; Requests - worker

void bdmworker::setSize(size_t newSize) {
    uint8_t *tmp;

    if ( newSize <= file.length )
        return;

    if ( (tmp = (uint8_t*)realloc( file.buffer, newSize )) == nullptr ) {
        free( file.buffer );
        printf("Catastrophic error - Could not reallocate file buffer\n");
        core.castMessage("Catastrophic error - Could not reallocate file buffer\n");
        exit( EXIT_FAILURE );
    }

    file.buffer = tmp;
    file.length = newSize;
}

void bdmworker::setRange(const memory_t *mem) {
    memory.startAt  = mem->address;
    memory.expectAt = mem->address;
    memory.stopAt   = mem->address + mem->size;
}

void bdmworker::reset() {

    timer.set( GLOBALTIMEOUT * 1000 );
    memory.autoProgress = true;

    // - mutex?
    // - - Received commands will currently lock out further reception by setting inFlight to false
    // - - Timeouts is where this could cause trouble

    // - inFlight  = false
    // - lastFault = RET_OK
    // - queue tx counters 0
    // - stored rx count 0
    queue.reset();

    resetRxTmp();
    
    file.location = 0;
}

bool bdmworker::getData( uint16_t *buf, Master_Commands toCmd, uint32_t size, uint32_t index ) {

    uint16_t *ptr     = rxQueue.buffer;
    uint32_t  seenCmd = 0;

    for (uint32_t i = 0; i < rxQueue.nStoredCommands; i++) {

        // Payload [ [cmd], [cmd len], [data (if present)] ] .. next
        uint16_t cmd = ptr[ 0 ];
        uint32_t len = ptr[ 1 ];

        if ( len < 2 ) {
            core.castMessage("Error: bdmworker::getData() - Malformed length");
            return false;
        }

        if ( cmd == toCmd ) {
            if ( seenCmd == index ) {
                if ( size == ((len - 2)*2) ) {
                    if ( size > 0 )
                        memcpy((void*)buf, (void*)&ptr[2], (size_t)size);
                    return true;
                } else {
                    // There will be no match after this point
                    return false;
                }
            }

            // Increment matching command count
            seenCmd++;
        }

        ptr += len;
    }

    return false;
}

void bdmworker::setTimeout(uint32_t msTimeout) {
    timer.set( msTimeout );
}

void bdmworker::autoProgress(bool state) {
    memory.autoProgress = state;
}

uint16_t bdmworker::lastFault() {
    return flags.lastFault;
}

bool bdmworker::swapBuffer(uint32_t blockSize) {
    uint8_t *fBuf = (uint8_t *)file.buffer;
    size_t   nBytes = file.location;
    uint8_t  swapBuffer[ 16 ];

    if ( blockSize < 2 || blockSize > sizeof(swapBuffer) ) {
        core.castMessage("Error: bdmworker::swapBuffer() - Unable to swap in steps of size %u", blockSize);
        return false;
    }

    if ( nBytes < blockSize ) {
        core.castMessage("Error: bdmworker::swapBuffer() - Not enough data in buffer");
        return false;
    }

    if ( (nBytes % blockSize) != 0 ) {
        core.castMessage("Error: bdmworker::swapBuffer() - Buffer size not in multiples of swapSize");
        return false;
    }

    while ( nBytes > 0 ) {
        fBuf = &fBuf[ blockSize ];
        for (uint32_t i = 0; i < blockSize; i++)
            swapBuffer[ i ] = *--fBuf;
        for (uint32_t i = 0; i < blockSize; i++)
            *fBuf++ = swapBuffer[ i ];
        nBytes -= blockSize;
    }

    return true;
}

// Compiler optimizations, ofstream and mingw == someone set off a nuke!
/*
bool bdmworker::saveFile(const char *fName) {

    std::ofstream fS;

    if ( file.location == 0 ) {
        core.castMessage("Error: bdmworker::saveFile() - There's no data to store");
        return false;
    }

    fS.open( fName, std::ios::binary );

    if ( !fS ) {
        core.castMessage("Error: bdmworker::saveFile() - Could not get file handle");
        return false;
    }

    fS.clear();
    fS.write( (char*)file.buffer, file.location );
    fS.flush();
    fS.close();

    if ( !fS ) {
        core.castMessage("Error: bdmworker::saveFile() - Could not write file");
        return false;
    }

    core.castMessage("Info: bdmworker::saveFile() - done");
    return true;
}
*/

bool bdmworker::saveFile(const char *fName) {

    FILE *fp;

    core.castMessage("Info: Storing file..");

    if ( file.location == 0 ) {
        core.castMessage("Error: bdmworker::saveFile() - There's no data to store");
        return false;
    }

    if ( (fp = fopen( fName, "wb" )) == nullptr ) {
        core.castMessage("Error: bdmworker::saveFile() - Could not get file handle");
        return false;
    }

    rewind(fp);

    size_t actWrite = fwrite(file.buffer, 1, file.location, fp);

    if ( actWrite != file.location ) {
        core.castMessage("Error: bdmworker::saveFile() - Could not store all the data");
        fclose(fp);
        return false;
    }

    fclose(fp);

    core.castMessage("Info: Done");

    return true;
}