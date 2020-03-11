#ifndef __HOME_SECURITY_H__
#define __HOME_SECURITY_H__

#include <chrono>
#include <glib.h>
#include <thread>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <curl/curl.h>
#include <jetson-utils/imageIO.h>
#include <jetson-utils/gstCamera.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>
#include "hsDetection.h"
#include "hsEmailMessage.h"
#include "hsFTP.h"
#include "hsRTSP.h"

using namespace std;

static bool program_running = true;

static const char *capture_subdir = "cap";
static const char *uploads_subdir = "upl";
static const char *extension_photo = "_jn.jpeg";
static const char *delimiter = "\n";

static const string& gotodetection = "./gotodetection";
static const string& gotostreaming = "./gotostreaming";

enum States {
    PREPARE_DETECTION,
    DETECTION,
    PREPARE_STREAMING,
    STREAMING,
    STOPPING
};

struct MemoryStruct {
    char *memory;
    size_t size;
};

inline bool hsFileExist (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
};

#endif /* __HOME_SECURITY_H__ */
