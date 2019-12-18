#include "emailMessage.h"

// constructor
emailMessage::emailMessage( const int numDetections,detectNet::Detection* detections ) {

    /* Fill the Header with dynamic data */
    mHeader.push_back( "Date: Mon, 16 Dec 2019 14:08:43 +0100" );
    mHeader.push_back( "To: " TO );
    mHeader.push_back( "From: " FROM " (example User)" );
    mHeader.push_back( "Cc: " CC " (Another example User)" );
    mHeader.push_back( "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>" );
    mHeader.push_back( "Subject: example sending a MIME-formatted message" );

    /* Fill the Inline Text with dynamic data */
    mInlineText.push_back( INLINE_TEXT );

    /* Fill the Inline HTML with dynamic data */
    mInlineHTML.push_back( INLINE_HTML );

    //printf("\nEMAIL: %i objects detected", numDetections);
    //printf("\nEMAIL: detected obj class #%u, confidence=%f", detections[0].ClassID, detections[0].Confidence);
}

// destructor
emailMessage::~emailMessage() {
}

// Print header
void emailMessage::printHeader( void ) {
    print( mHeader );
}

// Print header
void emailMessage::printInlineText( void ) {
    print( mInlineText );
}

// Print header
void emailMessage::printInlineHTML( void ) {
    print( mInlineHTML );
}

// Internal print function
void emailMessage::print( std::vector<std::string> vs ) {

    // Print Strings stored in Vector
    std::cout << std::endl;
    for( int i = 0; i < vs.size(); i++ ) 
        std::cout << vs[i] << std::endl; 
}
