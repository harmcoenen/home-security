#ifndef __EMAIL_MESSAGE_H__
#define __EMAIL_MESSAGE_H__

#include <jetson-utils/commandLine.h>
#include <jetson-inference/detectNet.h>


#define SMTP_URL "smtp://smtp.ziggo.nl"
#define FROM    "<jetson@familiecoenen.nl>"
#define TO      "<harm@familiecoenen.nl>"
#define CC      "<info@familiecoenen.nl>"

class emailMessage {

    /*
    *   Dynamic parts of email are:
    *       - Date:
    *       - Message-ID:
    *       - Subject:
    */

public:
    emailMessage(const int, detectNet::Detection*);
    ~emailMessage();
//    setHeader();

//private:
//    headerSetDate();
//    headerSetMessageID();
//    headerSetSubject();
};

#endif /* __EMAIL_MESSAGE_H__ */
