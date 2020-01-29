#ifndef __HOME_SECURITY_H__
#define __HOME_SECURITY_H__

#include <chrono>
#include <glib.h>
#include <thread>
#include <signal.h>
#include <jetson-utils/imageIO.h>
#include <jetson-utils/gstCamera.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>

#include "hsDetection.h"
#include "emailMessage.h"

using namespace std;

bool signal_recieved = false;
bool program_running = true;

#endif /* __HOME_SECURITY_H__ */
