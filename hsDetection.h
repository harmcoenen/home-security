#ifndef __HS_DETECTION_H__
#define __HS_DETECTION_H__

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>

//using namespace std;

#define TIME_SLICE_DURATION 60

class hsDetection {

    /* Member Variables */
    bool mActive;
    bool mEmailAllowed;
    time_t mSlicetime;
    double mDuration;

public:
    hsDetection();
    ~hsDetection();

    void setActive( bool );
    bool isActive( void );
    void setEmailAllowed( bool );
    bool isEmailAllowed( void );
    void resetSlicetime( void );
    double getDuration( void );
};

#endif /* __HS_DETECTION_H__ */
