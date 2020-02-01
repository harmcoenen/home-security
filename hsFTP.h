#ifndef __HS_FTP_H__
#define __HS_FTP_H__

#include <ctime>
#include <string>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include "home-security.h"

using namespace std;

/* ftp.familiecoenen.nl port 21, /public/www/recordings */
static const char *remote_url = "ftp://ftp.familiecoenen.nl/";

static size_t readCallback( void*, size_t, size_t, FILE* );
static size_t writeMemoryCallback( void*, size_t, size_t, void*);

class hsFTP {

    /* Member Variables */
    string mCredentials;
    string mRemoteDir;

public:
    hsFTP();
    ~hsFTP();

    void setCredentials( const char*, const char* );
    const char* getCredentials( void );
    void setRemoteDir( void );
    const char* getRemoteDir( void );
    int uploadFiles( void );
};


#endif /* __HS_FTP_H__ */
