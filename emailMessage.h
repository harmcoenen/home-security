#ifndef __EMAIL_MESSAGE_H__
#define __EMAIL_MESSAGE_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <jetson-utils/filesystem.h>
#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>


#define SMTP_URL "smtp://smtp.ziggo.nl"
#define FROM    "<jetson@familiecoenen.nl>"
#define TO      "<harm@familiecoenen.nl>"
#define CC      "<info@familiecoenen.nl>"

#define INLINE_TEXT "This is the inline text message of the e-mail.\r\n" \
                    "It could be a lot of lines that would be displayed " \
                    "in an e-mail viewer that is not able to handle HTML."

#define INLINE_HTML "<html><body>\r\n" \
                    "<p>This is the inline <b>HTML</b> message of the e-mail.</p>" \
                    "\r\n<br><br>\r\n" \
                    "<p>It could be a lot of HTML data that would be displayed by " \
                    "e-mail viewers able to handle HTML.</p>\r\n" \
                    "</body></html>\r\n"

class emailMessage {

    /* Member Variables */
    std::vector<std::string> mHeader;
    std::vector<std::string> mInlineText;
    std::vector<std::string> mInlineHTML;
    const char* attachment;

public:
    emailMessage( const int, detectNet::Detection*, const char* );
    ~emailMessage();

    int send( void );

    void printHeader( void );
    void printInlineText( void );
    void printInlineHTML( void );

private:
    void print( std::vector<std::string> );

};

#endif /* __EMAIL_MESSAGE_H__ */
