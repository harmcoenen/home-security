#include "hsFTP.h"

// constructor
hsFTP::hsFTP() {
}

// destructor
hsFTP::~hsFTP() {
}

void hsFTP::setCredentials( const char* username, const char* password ) {
    mCredentials.assign( username );
    mCredentials.append(":");
    mCredentials.append( password );
}

const char* hsFTP::getCredentials( void ) {
    return( mCredentials.c_str() );
}
