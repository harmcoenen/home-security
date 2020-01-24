#include "hsDetection.h"

// constructor
hsDetection::hsDetection() {
    time( &mStarttime );
    mActive = false;
}

// destructor
hsDetection::~hsDetection() {
    mActive = false;
}

void hsDetection::setActive( bool active ) {
    mActive = active;
}

bool hsDetection::isActive( void ) {
    return( mActive );
}

void hsDetection::setStarttime( void ) {
    time( &mStarttime );
}

double hsDetection::getDuration( void ) {
    time_t now;
    time( &now );

    return( difftime( now, mStarttime ) );
}