#ifndef __HOME_SECURITY_H__
#define __HOME_SECURITY_H__

#include <chrono>
#include <glib.h>
#include <thread>
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
#include "hsRSTP.h"

using namespace std;

static bool signal_recieved = false;
static bool program_running = true;

static const char *capture_subdir = "cap";
static const char *uploads_subdir = "upl";
static const char *extension_photo = "_jn.jpeg";
static const char *delimiter = "\n";

struct MemoryStruct {
    char *memory;
    size_t size;
};

#endif /* __HOME_SECURITY_H__ */
