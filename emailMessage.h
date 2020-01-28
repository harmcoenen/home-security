#ifndef __EMAIL_MESSAGE_H__
#define __EMAIL_MESSAGE_H__

#include <ctime>
#include <math.h>
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
#define MARKER   "@@_<x>_@@"
#define SUBJECT  "Subject: home-security has " MARKER " object(s) detected"
#define MAX_TIME_STRING 80


#define INLINE_TEXT "home-security has detected one or more objects.\r\n" \
                    "Here is an overview of the detection(s):\r\n" \
                    MARKER "\r\n"

#define INLINE_HTML "<html>\r\n" \
                    "<head>\r\n" \
                    "<style>\r\n" \
                    "table {\r\n" \
                    "  font-family: arial, sans-serif;\r\n" \
                    "  border-collapse: collapse;\r\n" \
                    "  width: 100%;\r\n" \
                    "}\r\n" \
                    "td, th {\r\n" \
                    "  border: 1px solid #dddddd;\r\n" \
                    "  text-align: left;\r\n" \
                    "  padding: 8px;\r\n" \
                    "}\r\n" \
                    "tr:nth-child(even) {\r\n" \
                    "  background-color: #dddddd;\r\n" \
                    "}\r\n" \
                    "</style>\r\n" \
                    "</head>\r\n" \
                    "<body>\r\n" \
                    "<h2>home-security has detected one or more objects.</h2>\r\n" \
                    "<br><br>\r\n" \
                    "<h3>Here is an overview of the detection(s):</h3>\r\n" \
                    MARKER "\r\n" \
                    "</body>\r\n" \
                    "</html>\r\n"

class emailMessage {

    /* Member Variables */
    vector<string> mHeader;
    string mSubject;
    string mBaseInlineText;
    string mBaseInlineHTML;
    const char* mAttachment;
    char mTimeString [MAX_TIME_STRING];

public:
    emailMessage( const int, detectNet::Detection*, detectNet*, const char* );
    ~emailMessage();

    int send( void );

    void printHeader( void );
    void printInlineText( void );
    void printInlineHTML( void );

private:
    size_t timeFormatted( void );
    string dynamicText( const int, detectNet::Detection*, detectNet* );
    string dynamicHTML( const int, detectNet::Detection*, detectNet* );
};

#endif /* __EMAIL_MESSAGE_H__ */
