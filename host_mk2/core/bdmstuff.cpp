//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// - - -
#include "bdmstuff.h"

void sleep_ms(uint32_t waitms) {
	auto oldTime = std::chrono::system_clock::now();
    auto newTime = oldTime;
	do {
		newTime = std::chrono::system_clock::now();
	} while (std::chrono::duration_cast<std::chrono::milliseconds>(newTime - oldTime).count() < waitms);
}

void bdmstuff::unload() {
    if ( target != nullptr ) {
        target->~iTarget();
        target = nullptr;
        desc = nullptr;
    }
}

// Load target
bool bdmstuff::load( int32_t idx ) {

    unload();

    // Reset worker and its queue
    bdmworker::reset();

    if ( idx < 0 || idx >= numTargets ) {
        castMessage("Error: Index out of bounds");
        return false;
    }

    if ( targets[ idx ]->instantiator == nullptr ) {
        castMessage("Error: No instantiator present");
        return false;
    }

    if ( (target = targets[ idx ]->instantiator( *this )) == nullptr ) {
        castMessage("Error: Could not instantiate target");
        return false;
    }

    desc = targets[ idx ];

    if ( targets[ idx ]->name != nullptr && targets[ idx ]->name[0] != 0 )
        castMessage("Loaded target %s", targets[ idx ]->name);
    else
        castMessage("Loaded anonymous target");

    return true;
}

// Return name of loaded target
const char *bdmstuff::name() {
    if ( desc == nullptr )
        return "Not loaded";
    if ( desc->name == nullptr || desc->name[0] == 0 )
        return "Not named";
    return desc->name;
}

// Return name of loaded target
const target_t *bdmstuff::info() {
    if ( desc == nullptr )
        return &errDesc;
    return desc;
}

bool bdmstuff::read( uint32_t region ) {

    stopwatch tim;
    bool status;

    bdmworker::reset();

    if ( target == nullptr ) {
        castMessage("Error: read() - There is no target loaded");
        return false;
    }

    if ( region >= desc->regions ) {
        castMessage("Error: read() - No such region");
        return false;
    }

    if ( bdmworker::usb::connect() == false )
        return false;

    tim.capture();

    if ( (status = target->init( desc, &desc->region[ region ] )) == true )
        status = target->read( desc, &desc->region[ region ] );

    tim.capture();

    bdmworker::usb::disconnect();

    castMessage("Info: spent %02d:%02d:%03d", tim.minutes, tim.seconds, tim.milliseconds);

    return status;
}
