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
#include "home-security.h"

void sig_handler(int signo)
{
    if( signo == SIGINT )
    {
        cout << "home-security: received SIGINT" << endl;
        signal_recieved = true;
    }
}

int usage()
{
    cout << "usage: home-security [--help] [--network NETWORK] [--threshold=THRESHOLD]" << endl;
    cout << "                     [--camera CAMERA] [--width=WIDTH] [--height=HEIGHT]" << endl << endl;
    cout << "Locate objects in a live camera stream using an object detection DNN." << endl << endl;
    cout << "Arguments:" << endl;
    cout << "  --help            show this help message and exit" << endl;
    cout << "  --network NETWORK pre-trained model to load (see below for options)" << endl;
    cout << "  --overlay OVERLAY detection overlay flags (e.g. --overlay=box,labels,conf)" << endl;
    cout << "                    valid combinations are:  'box', 'labels', 'conf', 'none'" << endl;
    cout << "  --alpha ALPHA     overlay alpha blending value, range 0-255 (default: 120)" << endl;
    cout << "  --camera CAMERA   index of the MIPI CSI camera to use (e.g. CSI camera 0)," << endl;
    cout << "                    or for VL42 cameras the /dev/video device to use." << endl;
    cout << "                    by default, MIPI CSI camera 0 will be used." << endl;
    cout << "  --width=WIDTH     desired width of camera stream (default is 1280 pixels)" << endl;
    cout << "  --height=HEIGHT   desired height of camera stream (default is 720 pixels)" << endl;
    cout << "  --threshold=VALUE minimum threshold for detection (default is 0.5)" << endl << endl;

    cout << detectNet::Usage() << endl;

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
        cout << "home-security: can't catch SIGINT" << endl;

    /*
     * create the camera device
     */
    gstCamera* camera = gstCamera::Create(cmdLine.GetInt("width", gstCamera::DefaultWidth),
                                          cmdLine.GetInt("height", gstCamera::DefaultHeight),
                                          cmdLine.GetString("camera"));

    if( !camera )
    {
        cout << "home-security: failed to initialize camera device" << endl;
        return 0;
    }
    
    cout << "home-security: successfully initialized camera device" << endl;
    cout << "    width:  " << camera->GetWidth() << endl;
    cout << "   height:  " << camera->GetHeight() << endl;
    cout << "    depth:  " << camera->GetPixelDepth() << " (bpp)" << endl;

    /*
     * create detection network
     */
    detectNet* net = detectNet::Create(argc, argv);
    
    if( !net )
    {
        cout << "home-security: failed to load detectNet model" << endl;
        return 0;
    }

    cout << "home-security: threshold is [" << net->GetThreshold() << "]" << endl;
    cout << "home-security: max detections is [" << net->GetMaxDetections() << "]" << endl;

    /*
     * parse overlay flags
     */
    const uint32_t overlayFlags = detectNet::OverlayFlagsFromStr(cmdLine.GetString("overlay", "box,labels,conf"));

    /*
     * start streaming
     */
    if( !camera->Open() )
    {
        cout << "home-security: failed to open camera for streaming" << endl;
        return 0;
    }

    cout << "home-security: camera open for streaming" << endl;

    /*
     * processing loop
     */
    while( !signal_recieved )
    {
        /*
         * capture RGBA image
         */
        float* imgRGBA = NULL;

        if( !camera->CaptureRGBA(&imgRGBA, 1000, true) ) /* timeout of 1000 msec and set zeroCopy to 'true' to access the image pixels from CPU */
            cout << "home-security: failed to capture RGBA image from camera" << endl;

        /*
         * detect objects in the frame
         */
        detectNet::Detection* detections = NULL;
    
        const int numDetections = net->Detect(imgRGBA, camera->GetWidth(), camera->GetHeight(), &detections, overlayFlags);
        
        if( numDetections > 0 )
        {
            /*
             * Print the number of detected objects
             */
            cout << "home-security: " << numDetections << " objects detected" << endl;

            int personFound = 0;
            for( int n=0; n < numDetections; n++ )
                if( strcmp( net->GetClassDesc( detections[n].ClassID ), "person" ) == 0 )
                    personFound++;

            /*
             * Save image to file and send it in an email, but only if a 'person' is detected
             */
            if( personFound ) {
                /*
                 * save image to jpeg file
                 */
                if( saveImageRGBA( detectedFilename, (float4*)imgRGBA, camera->GetWidth(), camera->GetHeight(), 255.0f, 100 ) ) {
                    cout << "home-security: saved (" << camera->GetWidth() << "x" << camera->GetHeight() << ") image to '" << detectedFilename << "'" << endl;
                } else {
                    cout << "home-security: failed saving (" << camera->GetWidth() << "x" << camera->GetHeight() << ") image to '" << detectedFilename << "'" << endl;
                }

                /*
                 * Construct dynamically a new email message and send it
                 */
                emailMessage email( numDetections, detections, net, detectedFilename );
                if( email.send() == CURLE_OK ) {
                    cout << "home-security: email sent." << endl;
                } else {
                    cout << "home-security: email send failed." << endl;
                }
            }
        }
    }

    /*
     * destroy resources
     */
    cout << "home-security:  shutting down..." << endl;

    SAFE_DELETE(camera);
    SAFE_DELETE(net);

    cout << "home-security:  shutdown complete." << endl;
    return 0;
}
