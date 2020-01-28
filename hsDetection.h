#ifndef __HS_DETECTION_H__
#define __HS_DETECTION_H__

#include <ctime>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <curl/curl.h>
#include <bits/stdc++.h> 
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>


using namespace std;

static const char *capture_subdir = "cap";
static const char *uploads_subdir = "upl";
static const char *extension_photo = "_jn.jpeg";

#define TIME_SLICE_DURATION 60 /* In seconds */

class hsDetection {

    /* Member Variables */
    bool mActive;
    bool mEmailAllowed;
    time_t mSlicetime;
    double mDuration;
    string mImageFilename;
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
    const char* getImageFilename( void );
};

#endif /* __HS_DETECTION_H__ */
