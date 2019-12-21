#ifndef __EMAIL_MESSAGE_H__
#define __EMAIL_MESSAGE_H__

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>

using namespace std;

#define SMTP_URL "smtp://smtp.ziggo.nl"
#define FROM     "<jetson@familiecoenen.nl>"
#define TO       "<harm@familiecoenen.nl>"
#define CC       "<info@familiecoenen.nl>"
#define DOT      "@@_DETECTION_OVERVIEW_TEXT_@@"
#define DOH      "@@_DETECTION_OVERVIEW_HTML_@@"
#define MAX_TIME_STRING 80


#define INLINE_TEXT "home-security has detected one or more objects.\r\n" \
                    "Here is an overview of the detection(s):\r\n" \
                    DOT "\r\n"

#define INLINE_HTML "<html><body>\r\n" \
                    "<p><b><u>home-security has detected one or more objects.</u></b></p>\r\n" \
                    "<br><br>\r\n" \
                    "<p>Here is an overview of the detection(s):</p>\r\n" \
                    "<p>" DOH "</p>\r\n" \
                    "</body></html>\r\n"

class emailMessage {

    /* Member Variables */
    vector<string> mHeader;
    vector<string> mInlineText;
    vector<string> mInlineHTML;
    const char* mAttachment;
    char mTimeString [MAX_TIME_STRING];

public:
    emailMessage( const int, detectNet::Detection*, const char* );
    ~emailMessage();

    int send( void );

    void printHeader( void );
    void printInlineText( void );
    void printInlineHTML( void );

private:
    void print( vector<string> );
    size_t timeFormatted( void );
};

#endif /* __EMAIL_MESSAGE_H__ */
