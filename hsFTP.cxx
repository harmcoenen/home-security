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

int hsFTP::uploadFiles() {
    DIR *dr = NULL;

    cout << "home-security: uploadFiles triggered" << endl;
    dr = opendir( uploads_subdir ); 
    if (dr == NULL) {
        cerr << "home-security: could not open directory of files to upload" << endl;
        return -1;
    } else {
        cout << "home-security: upload directory opened" << endl;
    }

}