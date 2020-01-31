#ifndef __HS_FTP_H__
#define __HS_FTP_H__

#include <ctime>
#include <string>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>

using namespace std;

#define FTP_URL "ftp://ftp.familiecoenen.nl/"

class hsFTP {

    /* Member Variables */
    string mCredentials;

public:
    hsFTP();
    ~hsFTP();

    void setCredentials( const char*, const char* );
    const char* getCredentials( void );
};


#endif /* __HS_FTP_H__ */
