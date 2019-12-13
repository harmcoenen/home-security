/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <jetson-utils/gstCamera.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>

#include <signal.h>


bool signal_recieved = false;

void sig_handler(int signo)
{
    if( signo == SIGINT )
    {
        printf("\nreceived SIGINT");
        signal_recieved = true;
    }
}

int usage()
{
    printf("usage: home-security [--help] [--network NETWORK] [--threshold=THRESHOLD]\n");
    printf("                     [--camera CAMERA] [--width=WIDTH] [--height=HEIGHT]\n\n");
    printf("Locate objects in a live camera stream using an object detection DNN.\n\n");
    printf("Arguments:\n");
    printf("  --help            show this help message and exit\n");
    printf("  --network NETWORK pre-trained model to load (see below for options)\n");
    printf("  --overlay OVERLAY detection overlay flags (e.g. --overlay=box,labels,conf)\n");
    printf("                    valid combinations are:  'box', 'labels', 'conf', 'none'\n");
    printf("  --alpha ALPHA     overlay alpha blending value, range 0-255 (default: 120)\n");
    printf("  --camera CAMERA   index of the MIPI CSI camera to use (e.g. CSI camera 0),\n");
    printf("                    or for VL42 cameras the /dev/video device to use.\n");
    printf("                    by default, MIPI CSI camera 0 will be used.\n");
    printf("  --width=WIDTH     desired width of camera stream (default is 1280 pixels)\n");
    printf("  --height=HEIGHT   desired height of camera stream (default is 720 pixels)\n");
    printf("  --threshold=VALUE minimum threshold for detection (default is 0.5)\n\n");

    printf("%s\n", detectNet::Usage());

    return 0;
}

int main( int argc, char** argv )
{
    /*
     * parse command line
     */
    commandLine cmdLine(argc, argv);

    if( cmdLine.GetFlag("help") )
        return usage();


    /*
     * attach signal handler
     */
    if( signal(SIGINT, sig_handler) == SIG_ERR )
        printf("\ncan't catch SIGINT");


    /*
     * create the camera device
     */
    gstCamera* camera = gstCamera::Create(cmdLine.GetInt("width", gstCamera::DefaultWidth),
                                          cmdLine.GetInt("height", gstCamera::DefaultHeight),
                                          cmdLine.GetString("camera"));

    if( !camera )
    {
        printf("\nhome-security: failed to initialize camera device\n");
        return 0;
    }
    
    printf("\nhome-security: successfully initialized camera device");
    printf("\n    width:  %u", camera->GetWidth());
    printf("\n   height:  %u", camera->GetHeight());
    printf("\n    depth:  %u (bpp)\n", camera->GetPixelDepth());
    

    /*
     * create detection network
     */
    detectNet* net = detectNet::Create(argc, argv);
    
    if( !net )
    {
        printf("\nhome-security: failed to load detectNet model\n");
        return 0;
    }

    printf("\nhome-security:  threshold is [%f]", net->GetThreshold());
    printf("\nhome-security:  max detections is [%i]", net->GetMaxDetections());


    // parse overlay flags
    const uint32_t overlayFlags = detectNet::OverlayFlagsFromStr(cmdLine.GetString("overlay", "box,labels,conf"));
    

    /*
     * start streaming
     */
    if( !camera->Open() )
    {
        printf("\nhome-security:  failed to open camera for streaming\n");
        return 0;
    }
    
    printf("\nhome-security:  camera open for streaming");
    
    
    /*
     * processing loop
     */
    while( !signal_recieved )
    {
        // capture RGBA image
        float* imgRGBA = NULL;
        
        if( !camera->CaptureRGBA(&imgRGBA, 1000) )
            printf("\nhome-security:  failed to capture RGBA image from camera");

        // detect objects in the frame
        detectNet::Detection* detections = NULL;
    
        const int numDetections = net->Detect(imgRGBA, camera->GetWidth(), camera->GetHeight(), &detections, overlayFlags);
        
        if( numDetections > 0 )
        {
            printf("\n%i objects detected", numDetections);
        
            for( int n=0; n < numDetections; n++ )
            {
                printf("\ndetected obj %i  class #%u (%s)  confidence=%f", n, detections[n].ClassID, net->GetClassDesc(detections[n].ClassID), detections[n].Confidence);
                printf("\nbounding box %i  (%f, %f)  (%f, %f)  w=%f  h=%f", n, detections[n].Left, detections[n].Top, detections[n].Right, detections[n].Bottom, detections[n].Width(), detections[n].Height()); 
            }
        }   

    }

    /*
     * destroy resources
     */
    printf("\nhome-security:  shutting down...");
    
    SAFE_DELETE(camera);
    //SAFE_DELETE(display);
    SAFE_DELETE(net);

    printf("\nhome-security:  shutdown complete.\n");
    return 0;
}
