#ifndef __HS_RTSP_H__
#define __HS_RTSP_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <jetson-utils/gstUtility.h>
#include <jetson-utils/gstCamera.h>
#include <iostream>
#include <sstream> 
#include <unistd.h>
#include <string.h>

using namespace std;

class hsRTSP {

public:
    static hsRTSP* Create( const char* username,
                           const char* password,
                           const char* ipaddr,
                           const char* port,
                           uint32_t width,
                           uint32_t height,
                           const char* camera=NULL );
    static hsRTSP* Create( const char* username,
                           const char* password,
                           const char* camera=NULL );
    ~hsRTSP();
    const char* getLaunchStr( void );

    static constexpr const char* DefaultRtspIpAddress = "192.168.178.249";
    static constexpr const char* DefaultRtspPort = "8554";
    static const uint32_t DefaultWidth  = 1280;
    static const uint32_t DefaultHeight = 720;

private:
    hsRTSP();
    bool init( gstCameraSrc src );
    bool buildLaunchStr( gstCameraSrc src );
    bool parseCameraStr( const char* camera );

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mDepth;
    uint32_t mSize;

    string mUserName;
    string mPassWord;
    string mIpAddress;
    string mPort;

    gstCameraSrc mSource;

    string  mLaunchStr;
    string  mCameraStr;

    int mSensorCSI;
    inline bool csiCamera() const { return ( mSensorCSI >= 0 ); }
};

#endif /* __HS_RTSP_H__ */
