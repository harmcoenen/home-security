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

void sig_handler( int signo )
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
    cout << "  --user=<username> Username for uploading images to FTP server and RTSP streaming" << endl;
    cout << "  --password=<pwd>  Password for uploading images to FTP server and RTSP streaming" << endl;
    cout << "  --ipaddr=<ipaddr> IP address of eth0 interface to be used for RTSP stream" << endl;
    cout << "  --port=<port>     Port number for RTSP stream broadcast" << endl;
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

void ftpUploadLoop( const char* username, const char* password ) {
    int progress = 0;
    int boundary = 60; /* 60 seconds is one minute */
    /*
     * Setup FTP and its credentials
     */
    hsFTP hs_ftp_upload;
    hs_ftp_upload.setCredentials( username, password );
    cout << "home-security: Credentials for ftpUploadLoop are [" << hs_ftp_upload.getCredentials() << "]" << endl;

    /*
     * FTP Upload main processing loop
     */
    while( program_running ) {

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

        progress++;
        if( (progress % boundary) == 0 ) {
            progress = 0;
            cout << "home-security: Uploaded " << hs_ftp_upload.uploadFiles() << " files" << endl;
        }
    }

    cout << "home-security: ftpUploadLoop end" << endl;
}

void ftpCleanupLoop( const char* username, const char* password ) {
    int progress = 0;
    int boundary = 1800; /* 1800 seconds is half an hour */
    /*
     * Setup FTP and its credentials
     */
    hsFTP hs_ftp_cleanup;
    hs_ftp_cleanup.setCredentials( username, password );
    cout << "home-security: Credentials for ftpCleanupLoop are [" << hs_ftp_cleanup.getCredentials() << "]" << endl;

    /*
     * FTP Upload main processing loop
     */
    while( program_running ) {

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

        progress++;
        if( (progress % boundary) == 0 ) {
            progress = 0;
            hs_ftp_cleanup.cleanupRemote();
        }
    }

    cout << "home-security: ftpCleanupLoop end" << endl;
}

int countInterestingObjects( const int numDetections, detectNet::Detection* detections, detectNet* net ) {
    int interestingObjects = 0;

    /*
     * Loop through the detections to find a match with interesting objects
     */
    for( int n=0; n < numDetections; n++ )
        if( ( strcmp( net->GetClassDesc( detections[n].ClassID ), "person" ) == 0 ) ||
            ( strcmp( net->GetClassDesc( detections[n].ClassID ), "dog"    ) == 0 ) )
            interestingObjects++;

    /*
     * Print the number of detected objects
     */
    cout << "home-security: " << numDetections << " objects detected of which " << interestingObjects << " are interesting" << endl;

    return( interestingObjects );
}

int main( int argc, char** argv )
{
    /*
     * Parse command line
     */
    commandLine cmdLine( argc, argv );

    if( cmdLine.GetFlag( "help" ) )
        return usage();

    /*
     * Attach signal handler
     */
    if( signal( SIGINT, sig_handler ) == SIG_ERR )
        cout << "home-security: can't catch SIGINT" << endl;

    /*
     * Create the camera device
     */
    gstCamera* camera = gstCamera::Create( cmdLine.GetInt( "width", gstCamera::DefaultWidth ),
                                           cmdLine.GetInt( "height", gstCamera::DefaultHeight ),
                                           cmdLine.GetString( "camera" ) );

    if( !camera )
    {
        cerr << "home-security: failed to initialize camera device" << endl;
        return 0;
    }
    
    cout << "home-security: successfully initialized camera device" << endl;
    cout << "    width:  " << camera->GetWidth() << endl;
    cout << "   height:  " << camera->GetHeight() << endl;
    cout << "    depth:  " << camera->GetPixelDepth() << " (bpp)" << endl;

    /*
     * Create detection network
     */
    detectNet* net = detectNet::Create( argc, argv );
    
    if( !net )
    {
        cout << "home-security: failed to load detectNet model" << endl;
        return 0;
    }

    cout << "home-security: threshold is [" << net->GetThreshold() << "]" << endl;
    cout << "home-security: max detections is [" << net->GetMaxDetections() << "]" << endl;

    /*
     * Parse overlay flags
     */
    const uint32_t overlayFlags = detectNet::OverlayFlagsFromStr( cmdLine.GetString( "overlay", "box,labels,conf" ) );

    /*
     * Start streaming
     */
    if( !camera->Open() )
    {
        cerr << "home-security: failed to open camera for streaming" << endl;
        return 0;
    }

    cout << "home-security: camera open for streaming" << endl;

    hsDetection hs_detection;

    thread ftpUploadthread( ftpUploadLoop, cmdLine.GetString( "user", "user" ), cmdLine.GetString( "password", "password" ) );
    thread ftpCleanupthread( ftpCleanupLoop, cmdLine.GetString( "user", "user" ), cmdLine.GetString( "password", "password" ) );


    /*
     * Create the RTSP stream
     */
    hsRTSP* rtsp_stream = hsRTSP::Create( cmdLine.GetString( "user", "user" ),
                                          cmdLine.GetString( "password", "password" ),
                                          cmdLine.GetString( "ipaddr", hsRTSP::DefaultRtspIpAddress ),
                                          cmdLine.GetString( "port", hsRTSP::DefaultRtspPort ),
                                          cmdLine.GetInt( "width", hsRTSP::DefaultWidth ),
                                          cmdLine.GetInt( "height", hsRTSP::DefaultHeight ),
                                          cmdLine.GetString( "camera" )
                                          );
    if( !rtsp_stream )
    {
        cerr << "home-security: failed to create RTSP stream" << endl;
        return 0;
    }

    /*
     * Main processing loop
     */
    while( !signal_recieved )
    {
        /* 
         * Wait some time before each detection to keep the device cool
         */
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

        hs_detection.handleTimeSlice();

        /*
         * Capture RGBA image
         */
        float* imgRGBA = NULL;

        if( !camera->CaptureRGBA( &imgRGBA, 1000, true ) ) /* Timeout of 1000 msec and set zeroCopy to 'true' to access the image pixels from CPU */
            cerr << "home-security: failed to capture RGBA image from camera" << endl;

        /*
         * Detect objects in the frame
         */
        detectNet::Detection* detections = NULL;
    
        const int numDetections = net->Detect( imgRGBA, camera->GetWidth(), camera->GetHeight(), &detections, overlayFlags );
        
        if( numDetections > 0 ) {

            int interestingObjects = countInterestingObjects( numDetections, detections, net );

            /*
             * Save image to file and send it in an email, but only if something interesting is detected
             */
            if( interestingObjects ) {

                /*
                 * Save image to jpeg file
                 */
                hs_detection.setImageFilename();
                if( saveImageRGBA( hs_detection.getCapImageFilename(), (float4*)imgRGBA, camera->GetWidth(), camera->GetHeight(), 255.0f, 100 ) ) {
                    cout << "home-security: saved (" << camera->GetWidth() << "x" << camera->GetHeight() << ") image to '" << hs_detection.getCapImageFilename() << "'" << endl;
                } else {
                    cerr << "home-security: failed saving (" << camera->GetWidth() << "x" << camera->GetHeight() << ") image to '" << hs_detection.getCapImageFilename() << "'" << endl;
                }

                /*
                 * Construct dynamically a new email message and send it
                 */
                if( hs_detection.isEmailAllowed() ) {

                    hsEmailMessage email( numDetections, detections, net, hs_detection.getCapImageFilename() );
                    if( email.send() == CURLE_OK ) {
                        cout << "home-security: email sent." << endl;
                    } else {
                        cerr << "home-security: email send failed." << endl;
                    }

                    hs_detection.setEmailAllowed( false );
                }

                /*
                 * Move the captured image to the upload location
                 */
                if ( rename( hs_detection.getCapImageFilename(), hs_detection.getUplImageFilename() ) == -1 ) {
                    cerr << "home-security: Failed to move file [" << hs_detection.getCapImageFilename() << "] to [" << hs_detection.getUplImageFilename() << "]" << endl;
                } else {
                    cout << "home-security: Moved file [" << hs_detection.getCapImageFilename() << "] to [" << hs_detection.getUplImageFilename() << "]" << endl;
                }

            }
        }
    }

    /*
     * Synchronize threads, pauses until thread finishes
     */
    cout << "home-security:  shutting down...   Waiting for ftpMainLoop to end. This might take a minute." << endl;
    program_running = false;
    ftpUploadthread.join();
    ftpCleanupthread.join();

    /*
     * Destroy resources
     */
    SAFE_DELETE( rtsp_stream );
    SAFE_DELETE( camera );
    SAFE_DELETE( net );

    cout << "home-security:  shutdown complete." << endl;
    return 0;
}
