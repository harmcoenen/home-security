#ifndef __HS_DETECTION_H__
#define __HS_DETECTION_H__

#include <ctime>
#include <math.h>
#include <stdio.h>
#include <iomanip>
#include <vector>
#include <string>
#include <iostream>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <curl/curl.h>
#include <bits/stdc++.h> 
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>
#include "home-security.h"

using namespace std;

#define TIME_SLICE_DURATION 60 /* In seconds */

class hsDetection {

    /* Member Variables */
    bool mActive;
    bool mEmailAllowed;
    time_t mSlicetime;
    double mDuration;
    string mCapImageFilename;
    string mUplImageFilename;
    int mImageSequenceNumber;  /* may wrap around */

public:
    hsDetection();
    ~hsDetection();

    void setActive( bool );
    bool isActive( void );
    void setEmailAllowed( bool );
    bool isEmailAllowed( void );
    void resetSlicetime( void );
    double getDuration( void );
    void setImageFilename( void );
    const char* getCapImageFilename( void );
    const char* getUplImageFilename( void );
};

#endif /* __HS_DETECTION_H__ */
