#include "hsDetection.h"

// constructor
hsDetection::hsDetection() {
    time( &mSlicetime );
    mActive = true;
    mEmailAllowed = true;
}

// destructor
hsDetection::~hsDetection() {
    mActive = false;
    mEmailAllowed = false;
}

void hsDetection::setActive( bool active ) {
    mActive = active;
}

bool hsDetection::isActive( void ) {
    return( mActive );
}

void hsDetection::setEmailAllowed( bool allowed ) {
    mEmailAllowed = allowed;
}

bool hsDetection::isEmailAllowed( void ) {
    return( mEmailAllowed );
}

void hsDetection::resetSlicetime( void ) {
    time( &mSlicetime );
}

double hsDetection::getDuration( void ) {
    time_t now;
    time( &now );

    return( difftime( now, mSlicetime ) );
}