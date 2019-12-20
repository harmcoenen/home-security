#ifndef __HOME_SECURITY_H__
#define __HOME_SECURITY_H__

#include <jetson-utils/imageIO.h>
#include <jetson-utils/gstCamera.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>
#include <signal.h>
#include <glib.h>

#include "emailMessage.h"

using namespace std;

const char* outputFilename = "/home/nano/Pictures/picture.jpeg";

bool signal_recieved = false;

#endif /* __HOME_SECURITY_H__ */
