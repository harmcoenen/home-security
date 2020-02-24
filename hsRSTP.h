#ifndef __HS_RSTP_H__
#define __HS_RSTP_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>

using namespace std;

#define DEFAULT_RTSP_PORT "8554"
static char *port = (char *) DEFAULT_RTSP_PORT;

class hsRSTP {

public:
    static hsRSTP* Create();
    ~hsRSTP();

private:
    hsRSTP();
    bool init();
};

#endif /* __HS_RSTP_H__ */
