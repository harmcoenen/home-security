#include "hsDetection.h"

// constructor
hsDetection::hsDetection() {
    time( &mSlicetime );
    mActive = true;
    mEmailAllowed = true;
    mImageFilename.assign( "detected.jpeg" );
    mImageSequenceNumber = 1;
    // Creating directories
    mkdir( capture_subdir, 0777 );
    mkdir( uploads_subdir, 0777 );
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

void hsDetection::setImageFilename( void ) {
    time_t rawtime;
    struct tm * timeinfo;
    ostringstream oss;

    time( &rawtime );
    timeinfo = localtime( &rawtime );
    oss << capture_subdir << "/" << put_time( timeinfo, "%Y_%m_%d_%H_%M_%S" ) << "_" << mImageSequenceNumber++ << extension_photo;
    mImageFilename.assign( oss.str().c_str() );
}

const char* hsDetection::getImageFilename( void ) {
    return( mImageFilename.c_str() );
}
