#include "hsRTSP.h"

// Create
hsRTSP* hsRTSP::Create( const char* username,
                        const char* password,
                        const char* ipaddr,
                        const char* port,
                        uint32_t width,
                        uint32_t height,
                        const char* camera )
{
    hsRTSP* rtsp = new hsRTSP();

    if( !rtsp )
        return NULL;

    if( !rtsp->parseCameraStr( camera ) )
        return NULL;

    rtsp->mWidth     = width;
    rtsp->mHeight    = height;
    rtsp->mDepth     = rtsp->csiCamera() ? 12 : 24;
    rtsp->mSize      = (width * height * rtsp->mDepth) / 8;

    rtsp->mUserName  = username;
    rtsp->mPassWord  = password;
    rtsp->mIpAddress = ipaddr;
    rtsp->mPort      = port;

    /*
    cout << "hsRTSP: Going to use Username   : " << rtsp->mUserName.c_str() << endl;
    cout << "hsRTSP: Going to use Password   : " << rtsp->mPassWord.c_str() << endl;
    cout << "hsRTSP: Going to use IP Address : " << rtsp->mIpAddress.c_str() << endl;
    cout << "hsRTSP: Going to use Port       : " << rtsp->mPort.c_str() << endl;
    */

    if( !rtsp->init( GST_SOURCE_NVARGUS ) )
    {
        cerr << "hsRTSP: failed to init rtsp stream with GST_SOURCE_NVARGUS, " << rtsp->mCameraStr.c_str() << endl;

        if( !rtsp->init( GST_SOURCE_NVCAMERA ) )
        {
            cerr << "hsRTSP: failed to init rtsp stream with GST_SOURCE_NVCAMERA, " << rtsp->mCameraStr.c_str() << endl;

            if( rtsp->mSensorCSI >= 0 ) rtsp->mSensorCSI = -1;

            if( !rtsp->init( GST_SOURCE_V4L2 ) )
            {
                cerr << "hsRTSP: failed to init rtsp stream with GST_SOURCE_V4L2, " << rtsp->mCameraStr.c_str() << endl;
                return NULL;
            }
        }
    }

    cout << "hsRTSP: rtsp stream successfully initialized with " << rtsp->mCameraStr.c_str() << endl;
    return rtsp;
}

// Create with several defaults
hsRTSP* hsRTSP::Create( const char* username,
                        const char* password,
                        const char* camera )
{
    return Create( username, password, DefaultRtspIpAddress, DefaultRtspPort, DefaultWidth, DefaultHeight, camera );
}

// constructor
hsRTSP::hsRTSP()
{
    mWidth  = 0;
    mHeight = 0;
    mDepth  = 0;
    mSize   = 0;

    mSource = GST_SOURCE_NVCAMERA;

    mSensorCSI  = -1;
}

// destructor
hsRTSP::~hsRTSP()
{
}

// getLaunchStr
const char* hsRTSP::getLaunchStr( void )
{
    return( mLaunchStr.c_str() );
}


// init
bool hsRTSP::init( gstCameraSrc src )
{
    // build pipeline string
    if( !buildLaunchStr( src ) )
    {
        return false;
    }

    return true;
}

// buildLaunchStr
bool hsRTSP::buildLaunchStr( gstCameraSrc src )
{
    // gst-launch-1.0 nvcamerasrc fpsRange="30.0 30.0" ! 'video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)I420, framerate=(fraction)30/1' ! \
    // nvvidconv flip-method=2 ! 'video/x-raw(memory:NVMM), format=(string)I420' ! fakesink silent=false -v
    // #define CAPS_STR "video/x-raw(memory:NVMM), width=(int)2592, height=(int)1944, format=(string)I420, framerate=(fraction)30/1"
    // #define CAPS_STR "video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)I420, framerate=(fraction)30/1"
    std::ostringstream ss;

    if( csiCamera() && src != GST_SOURCE_V4L2 )
    {
        mSource = src;

    #if NV_TENSORRT_MAJOR > 1 && NV_TENSORRT_MAJOR < 5      // if JetPack 3.1-3.3 (different flip-method)
        const int flipMethod = 0;                           // Xavier (w/TRT5) camera is mounted inverted
    #else
        const int flipMethod = 2;
    #endif

        if( src == GST_SOURCE_NVCAMERA )
            ss << "nvcamerasrc fpsRange=\"30.0 30.0\" ! video/x-raw(memory:NVMM), width=(int)" << mWidth << ", height=(int)" << mHeight << ", format=(string)NV12 ! nvvidconv flip-method=" << flipMethod << " ! ";
        else if( src == GST_SOURCE_NVARGUS )
            ss << "nvarguscamerasrc sensor-id=" << mSensorCSI << " ! video/x-raw(memory:NVMM), width=(int)" << mWidth << ", height=(int)" << mHeight << ", framerate=30/1, format=(string)NV12 ! nvvidconv flip-method=" << flipMethod << " ! ";

        ss << "x264enc ! rtph264pay name=pay0 pt=96";
    }
    else
    {
        ss << "v4l2src device=" << mCameraStr << " ! ";
        ss << "video/x-raw, width=(int)" << mWidth << ", height=(int)" << mHeight << ", "; 

    #if NV_TENSORRT_MAJOR >= 5
        ss << "format=YUY2 ! videoconvert ! video/x-raw, format=RGB ! videoconvert !";
    #else
        ss << "format=RGB ! videoconvert ! video/x-raw, format=RGB ! videoconvert !";
    #endif

        ss << "x264enc ! rtph264pay name=pay0 pt=96";

        mSource = GST_SOURCE_V4L2;
    }

    mLaunchStr = ss.str();

    return true;
}

// parseCameraStr
bool hsRTSP::parseCameraStr( const char* camera )
{
    if( !camera || strlen( camera ) == 0 )
    {
        mSensorCSI = 0;
        mCameraStr = "0";
        return true;
    }

    mCameraStr = camera;

    // check if the string is a V4L2 device
    const char* prefixV4L2 = "/dev/video";
    const size_t prefixLength = strlen( prefixV4L2 );
    const size_t cameraLength = strlen( camera );

    if( cameraLength < prefixLength )
    {
        const int result = sscanf( camera, "%i", &mSensorCSI );

        if( result == 1 && mSensorCSI >= 0 )
            return true;
    }
    else if( strncmp( camera, prefixV4L2, prefixLength ) == 0 )
    {
        return true;
    }

    cerr << "hsRTSP: Invalid camera device requested by Create: " << camera << endl;
    return false;
}
