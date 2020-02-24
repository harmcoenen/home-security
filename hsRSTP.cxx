#include "hsRSTP.h"

// Create
hsRSTP* hsRSTP::Create()
{
    hsRSTP* rstp = new hsRSTP();

    if( !rstp )
        return NULL;

    if( rstp->init() ) {
        cout << "hsRSTP successfully initialized" << endl; 
    }

    return rstp;
}

// constructor
hsRSTP::hsRSTP()
{
}

// destructor	
hsRSTP::~hsRSTP()
{
}

// init
bool hsRSTP::init()
{
    return true;
}