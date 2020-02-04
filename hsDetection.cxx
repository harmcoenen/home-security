#include "hsDetection.h"

// constructor
hsDetection::hsDetection() {
    time( &mSlicetime );
    mActive = true;
    mEmailAllowed = true;
    mCapImageFilename.assign( "capture.jpeg" );
    mUplImageFilename.assign( "uploads.jpeg" );
    mImageSequenceNumber = 0;
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

void hsDetection::handleTimeSlice( void ) {
    time_t now;
    time( &now );

    if( difftime( now, mSlicetime ) > TIME_SLICE_DURATION ) {
        time( &mSlicetime );
        mEmailAllowed = true;
    }
}

void hsDetection::setEmailAllowed( bool allowed ) {
    mEmailAllowed = allowed;
}

bool hsDetection::isEmailAllowed( void ) {
    return( mEmailAllowed );
}

void hsDetection::setImageFilename( void ) {
    time_t rawtime;
    struct tm * timeinfo;
    ostringstream oss;

    /* Increment sequence number */
    mImageSequenceNumber++;

    /* Get local timestamp */
    time( &rawtime );
    timeinfo = localtime( &rawtime );

    /* Construct capture filename */
    oss << capture_subdir << "/" << put_time( timeinfo, "%Y_%m_%d_%H_%M_%S" ) << "_" << mImageSequenceNumber << extension_photo;
    mCapImageFilename.assign( oss.str().c_str() );
    
    /* Empty oss string and clear any error flags */
    oss.str(""); oss.clear();
    
    /* Construct upload filename */
    oss << uploads_subdir << "/" << put_time( timeinfo, "%Y_%m_%d_%H_%M_%S" ) << "_" << mImageSequenceNumber << extension_photo;
    mUplImageFilename.assign( oss.str().c_str() );
}

const char* hsDetection::getCapImageFilename( void ) {
    return( mCapImageFilename.c_str() );
}

const char* hsDetection::getUplImageFilename( void ) {
    return( mUplImageFilename.c_str() );
}
