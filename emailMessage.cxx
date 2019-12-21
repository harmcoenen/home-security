#include "emailMessage.h"

// constructor
emailMessage::emailMessage( const int numDetections, detectNet::Detection* detections, detectNet* net, const char* detectedFilename ) {

    string dynamicText;
    string dynamicHTML;
    string marker( MARKER );
    mSubject = SUBJECT;
    mBaseInlineText = INLINE_TEXT ;
    mBaseInlineHTML = INLINE_HTML ;

    /* Fill the Header with dynamic data */
    if ( timeFormatted() > 30 ) {
        mHeader.push_back( mTimeString );
    } else {
        mHeader.push_back( "Date: Wed, 01 Jan 2020 12:34:56 +0100" );
    }
    mHeader.push_back( "To: " TO );
    mHeader.push_back( "From: " FROM " Jetson-Nano" );
    mHeader.push_back( "Cc: " CC " Info-Box" );
    //mHeader.push_back( "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>" );
    mSubject.replace( mSubject.find( marker ), marker.length(), to_string( numDetections ) );
    mHeader.push_back( mSubject );

    /* Fill the Inline Text with dynamic data */
    dynamicText.append( "REPLACED TEXT OVERVIEW" );
    mBaseInlineText.replace( mBaseInlineText.find( marker ), marker.length(), dynamicText);

    /* Fill the Inline HTML with dynamic data */
    for( int n=0; n < numDetections; n++ )
    {
        dynamicHTML.append( "home-security: detected obj " );
        dynamicHTML.append( to_string( n ) );
        dynamicHTML.append( " class #" );
        dynamicHTML.append( to_string( detections[n].ClassID ) );
        dynamicHTML.append( " (" );
        dynamicHTML.append( net->GetClassDesc( detections[n].ClassID ) );
        dynamicHTML.append( ") confidence=" );
        dynamicHTML.append( to_string( detections[n].Confidence ) );
        dynamicHTML.append( "<br>");

        dynamicHTML.append( "bounding box " );
        dynamicHTML.append( to_string( n ) );
        dynamicHTML.append( " (" );
        dynamicHTML.append( to_string( detections[n].Left ) );
        dynamicHTML.append( ", " );
        dynamicHTML.append( to_string( detections[n].Top ) );
        dynamicHTML.append( ") (" );
        dynamicHTML.append( to_string( detections[n].Right ) );
        dynamicHTML.append( ", " );
        dynamicHTML.append( to_string( detections[n].Bottom ) );
        dynamicHTML.append( "), width=" );
        dynamicHTML.append( to_string( detections[n].Width() ) );
        dynamicHTML.append( ", height=" );
        dynamicHTML.append( to_string( detections[n].Height() ) );
        dynamicHTML.append( ", area=" );
        dynamicHTML.append( to_string( detections[n].Area() ) );
        dynamicHTML.append( "<br>");
    }
    mBaseInlineHTML.replace( mBaseInlineHTML.find( marker ), marker.length(), dynamicHTML);

    mAttachment = detectedFilename;
}

// destructor
emailMessage::~emailMessage() {
}


// Send the email
int emailMessage::send( void ) {
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        struct curl_slist *recipients = NULL;
        struct curl_slist *slist = NULL;
        struct curl_slist *alist = NULL;
        curl_mime *mime;
        curl_mime *alt;
        curl_mimepart *part;

        /* This is the URL for your mailserver */
        curl_easy_setopt(curl, CURLOPT_URL, SMTP_URL);

        /* Note that this option isn't strictly required, omitting it will result
         * in libcurl sending the MAIL FROM command with empty sender data. All
         * autoresponses should have an empty reverse-path, and should be directed
         * to the address in the reverse-path which triggered them. Otherwise,
         * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
         * details.
         */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

        /* Add two recipients, in this particular case they correspond to the
         * To: and Cc: addressees in the header, but they could be any kind of
         * recipient. */
        recipients = curl_slist_append(recipients, TO);
        recipients = curl_slist_append(recipients, CC);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        /* Build and set the message header list. */
        if( !mHeader.empty() ) {
            for( int i = 0; i < mHeader.size(); i++ )
                headers = curl_slist_append(headers, mHeader[i].c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        /* Build the mime message. */
        mime = curl_mime_init(curl);

        /* The inline part is an alternative proposing the html and the text
           versions of the e-mail. */
        alt = curl_mime_init(curl);

        /* HTML message. */
        part = curl_mime_addpart(alt);
        curl_mime_data(part, mBaseInlineHTML.c_str(), CURL_ZERO_TERMINATED);
        curl_mime_type(part, "text/html");

        /* Text message. */
        part = curl_mime_addpart(alt);
        curl_mime_data(part, mBaseInlineText.c_str(), CURL_ZERO_TERMINATED);

        /* Create the inline part. */
        part = curl_mime_addpart(mime);
        curl_mime_subparts(part, alt);
        curl_mime_type(part, "multipart/alternative");
        slist = curl_slist_append(NULL, "Content-Disposition: inline");
        curl_mime_headers(part, slist, 1);

        /* Add the current source program as an attachment. */
        if( fileExists( mAttachment ) ) {
            part = curl_mime_addpart(mime);
            curl_mime_filedata(part, mAttachment);
            curl_mime_type(part, "image/jpeg");
            curl_mime_encoder(part, "base64");
            alist = curl_slist_append(NULL, "Content-Transfer-Encoding: base64");
            curl_mime_headers(part, alist, 1);
        }

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        /* Send the message */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        /* Free lists. */
        curl_slist_free_all(recipients);
        curl_slist_free_all(headers);

        /* curl won't send the QUIT command until you call cleanup, so you should
         * be able to re-use this connection for additional messages (setting
         * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
         * curl_easy_perform() again. It may not be a good idea to keep the
         * connection open for a very long time though (more than a few minutes
         * may result in the server timing out the connection), and you do want to
         * clean up in the end.
         */
        curl_easy_cleanup(curl);

        /* Free multipart message. */
        curl_mime_free(mime);
    }

    return (int)res;
}

void emailMessage::printHeader( void ) {
    for( int i = 0; i < mHeader.size(); i++ )
        cout << mHeader[i] << endl;
}

void emailMessage::printInlineText( void ) {
    cout << mBaseInlineText << endl;
}

void emailMessage::printInlineHTML( void ) {
    cout << mBaseInlineHTML << endl;
}

size_t emailMessage::timeFormatted( void ) {
  time_t rawtime;
  struct tm * timeinfo;

  time( &rawtime );
  timeinfo = localtime( &rawtime );

  return( strftime( mTimeString, MAX_TIME_STRING, "Date: %a, %d %b %G %H:%M:%S %z", timeinfo ) );
}