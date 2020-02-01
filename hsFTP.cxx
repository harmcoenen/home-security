#include "hsFTP.h"

static size_t readCallback( void *ptr, size_t size, size_t nmemb, FILE *stream ) {
    size_t retcode = fread( ptr, size, nmemb, stream );
    return retcode;
}

static size_t writeMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp ) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char *)realloc( mem->memory, mem->size + realsize + 1 );
    if( ptr == NULL ) {
        /* out of memory! */
        cerr << "Not enough memory (realloc returned NULL)" << endl;
        return 0;
    }

    mem->memory = ptr;
    memcpy( &( mem->memory[mem->size] ), contents, realsize );
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// constructor
hsFTP::hsFTP() {
    mRemoteDir.assign( "2000.08.04-12hrs" );
}

// destructor
hsFTP::~hsFTP() {
}

void hsFTP::setCredentials( const char* username, const char* password ) {
    mCredentials.assign( username );
    mCredentials.append( ":" );
    mCredentials.append( password );
}

const char* hsFTP::getCredentials( void ) {
    return( mCredentials.c_str() );
}

void hsFTP::setRemoteDir( void ) {
    time_t rawtime;
    struct tm * timeinfo;
    ostringstream oss;

    /* Get local timestamp */
    time( &rawtime );
    timeinfo = localtime( &rawtime );

    /* Construct capture filename */
    oss << put_time( timeinfo, "%Y.%m.%d-%Hhrs" );
    mRemoteDir.assign( oss.str().c_str() );
}

const char* hsFTP::getRemoteDir( void ) {
    return( mRemoteDir.c_str() );
}

int hsFTP::uploadFiles( void ) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *fileptr;
    DIR *dirptr = NULL;
    struct stat fileinfo;
    struct dirent *de;
    string remotefile;
    string localfile;
    int n_uploaded_files = 0;

    /*
     * Open upload directory
     */
    dirptr = opendir( uploads_subdir ); 
    if( dirptr == NULL ) {
        cerr << "home-security: could not open directory of files to upload" << endl;
        return -1;
    }

    /*
     * Determine the new remote directory (date/time formatted)
     */
    setRemoteDir();

    curl_global_init( CURL_GLOBAL_ALL );

    /*
     * Get a curl handle
     */
    curl = curl_easy_init();
    if( curl ) {
        /*
         * Setup curl
         */
        curl_easy_setopt( curl, CURLOPT_READFUNCTION, readCallback );
        curl_easy_setopt( curl, CURLOPT_UPLOAD, 1L );
        curl_easy_setopt( curl, CURLOPT_USERPWD, getCredentials() );
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
        curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 1L );
        curl_easy_setopt( curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR );
        curl_easy_setopt( curl, CURLOPT_NEW_DIRECTORY_PERMS, 0755L ); /* Default is 0755 */
        curl_easy_setopt( curl, CURLOPT_NEW_FILE_PERMS, 0644L) ; /* Default is 0644 */

        /*
         * Loop through the upload directory and handle each file. Skip the . and ..
         */
        while( ( de = readdir( dirptr ) ) != NULL ) {
            if( (strcmp( ".", de->d_name ) != 0 ) && ( strcmp( "..", de->d_name ) != 0 ) ) {

                /*
                 * Setup remotefile and localfile strings for transfer
                 */
                remotefile.assign( remote_url );
                remotefile.append( getRemoteDir() );
                remotefile.append( "/" );
                remotefile.append( de->d_name );
                localfile.assign( uploads_subdir );
                localfile.append( "/" );
                localfile.append( de->d_name );

                //cout << "home-security: going to upload " << localfile.c_str() << " to " << remotefile.c_str() << endl;

                curl_easy_setopt( curl, CURLOPT_URL, remotefile.c_str() );

                /*
                 * Get the file size of the local file
                 */
                if( stat( localfile.c_str(), &fileinfo ) ) {
                    cerr << "home-security: could not open " << localfile.c_str() << endl;

                    /*
                     * Always cleanup
                     */
                    curl_easy_cleanup( curl );
                    curl_global_cleanup();
                    closedir( dirptr );
                    return -1;
                }

                //cout << "home-security: local file size: " << (curl_off_t)fileinfo.st_size << " bytes" << endl;

                /*
                 * Open the local file and add to the curl request
                 */
                fileptr = fopen( localfile.c_str(), "rb" );
                curl_easy_setopt( curl, CURLOPT_READDATA, fileptr );
                curl_easy_setopt( curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileinfo.st_size );

                /*
                 * Perform the custom curl request
                 */
                res = curl_easy_perform( curl );
                if( res == CURLE_OK ) {
                    n_uploaded_files++;
                    //cout << "home-security: file " << localfile.c_str() << " uploaded successfully" << endl;

                    /*
                     * Remove file if upload successfull
                     */
                    if( remove( localfile.c_str() ) == 0 ) {
                        //cout << "home-security: file " << localfile.c_str() << " deleted successfully" << endl;
                    } else {
                        cerr << "home-security: unable to delete " << localfile.c_str() << endl;
                    }
                } else {
                    cerr << "home-security: curl_easy_perform() failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
                }

                /*
                 * Close the local file
                 */
                fclose( fileptr );
            }
        }

        /*
         * Always cleanup
         */
        curl_easy_cleanup( curl );
    }

    /*
     * Close the directory and cleanup
     */
    closedir( dirptr );
    curl_global_cleanup();

    return( n_uploaded_files );
}
