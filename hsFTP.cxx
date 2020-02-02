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
    oss << put_time( timeinfo, dir_format );
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
        curl_easy_setopt( curl, CURLOPT_UPLOAD, 1L );
        curl_easy_setopt( curl, CURLOPT_USERPWD, getCredentials() );
        curl_easy_setopt( curl, CURLOPT_VERBOSE, 0L );
        curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 1L );
        curl_easy_setopt( curl, CURLOPT_READFUNCTION, readCallback );
        curl_easy_setopt( curl, CURLOPT_NEW_FILE_PERMS, 0644L) ; /* Default is 0644 */
        curl_easy_setopt( curl, CURLOPT_NEW_DIRECTORY_PERMS, 0755L ); /* Default is 0755 */
        curl_easy_setopt( curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR );

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

void hsFTP::cleanupRemote( void ) {
    struct MemoryStruct list;
    char *remote_dir_name;

    list.memory = (char*) malloc(1);   /* will be grown as needed by the realloc */
    list.size = 0;                     /* no data at this point */

    if( remoteListDirectory( &list ) == 0 ) {
        cout << "home-security: list size " << list.size << ", memory " << list.memory << endl;

        if( list.size > 0 ) {
            while( ( remote_dir_name = strsep( &list.memory, delimiter ) ) != NULL ) {
                if( strlen( remote_dir_name ) > 0) {
                    cout << "home-security: remote dir name is " << remote_dir_name << " (" << strlen( remote_dir_name ) << "/" << strlen( list.memory ) << ")" << endl;

                    if( strcmp( remote_dir_name, "detections.php" ) == 0 ) {
                        cout << "home-security: core file " << remote_dir_name << " will not be deleted" << endl;
                    } else if( strncmp( ".", remote_dir_name, 1 ) == 0 ) {
                        cout << "home-security: hidden item " << remote_dir_name << " will be ignored" << endl;
                    } else if( TRUE == retentionPeriodExpired( remote_dir_name ) ) {
                        remoteRemoveDirectory( remote_dir_name );
                        cout << "home-security: retention period for " << remote_dir_name << " was expired" << endl;
                    } else {
                        cout << "home-security: retention period for " << remote_dir_name << " is NOT yet expired" << endl;
                    }
                }
            }
        }
    }

    free(list.memory); // Probably not needed as the strsep function handles the memory for this.
}

bool hsFTP::retentionPeriodExpired( const char* remote_dir_name ) {
    gboolean retval = FALSE;
    struct tm tmp = {0}; // Initialization is needed to have all values set properly.
    time_t now, remote_dir_time;
    double delta_time;

    strptime( remote_dir_name, dir_format, &tmp);

    remote_dir_time = mktime( &tmp );
    if( remote_dir_time > 0 ) {
        time( &now );
        delta_time = difftime( now, remote_dir_time );
        cout << "home-security: delta-time is " << delta_time << ", retention period is " << RETENTION_PERIOD << ", now " << now << ", remote_dir_time " << remote_dir_time << endl;
        if( delta_time > RETENTION_PERIOD ) retval = TRUE;
    } else {
        // mktime could not convert the tm struct values to an epoch value
        // This usually happens when the directory name has a different format(i.e. not an auto generated date/time stamp format)
        cout << "home-security: remote dir " << remote_dir_name << " doesn't contain a valid date/time format " << dir_format << ", so don't delete it automatically" << endl;
        retval = FALSE;
    }

    return( retval );
}

int hsFTP::remoteListDirectory( void *list ) {
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT; /* By default expect curl to fail */

    curl_global_init( CURL_GLOBAL_ALL );

    /*
     * Get a curl handle
     */
    curl = curl_easy_init();
    if( curl ) {

        /*
         * Setup curl
         */
        curl_easy_setopt( curl, CURLOPT_URL, remote_url );
        curl_easy_setopt( curl, CURLOPT_USERPWD, getCredentials() );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, list ); /* we pass our 'list' struct to the callback function */
        curl_easy_setopt( curl, CURLOPT_USERAGENT, "libcurl-agent/1.0" ); /* some servers don't like requests that are made without a user-agent field, so we provide one */
        curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, "NLST" ); /* NLST or LIST */
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback ); /* send all data to this function */

        /*
         * Perform the custom curl request
         */
        res = curl_easy_perform( curl );
        if( res != CURLE_OK ) {
            cerr << "home-security: curl_easy_perform() failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
        }

        /*
         * Always cleanup
         */
        curl_easy_cleanup( curl );
    }

    /*
     * Always cleanup
     */
    curl_global_cleanup();

    return( (int)res);
}

int hsFTP::remoteRemoveDirectory( const char *remote_dir ) {
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT; /* By default expect curl to fail */
    struct MemoryStruct list;
    string remotedir;
    string remove_cmd;
    char errbuf[CURL_ERROR_SIZE] = "DEADBEEF";
    char *remote_file_name;

    list.memory = (char*) malloc(1);   /* will be grown as needed by the realloc */
    list.size = 0;                     /* no data at this point */

    curl_global_init( CURL_GLOBAL_ALL );

    /*
     * Get a curl handle
     */
    curl = curl_easy_init();
    if (curl) {

        /*
         * Setup curl
         */
        remotedir.assign( remote_url );
        remotedir.append( remote_dir );
        remotedir.append( "/" );
        curl_easy_setopt( curl, CURLOPT_URL, remotedir.c_str() );
        curl_easy_setopt( curl, CURLOPT_USERPWD, getCredentials() );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&list ); /* we pass our 'list' struct to the callback function */
        curl_easy_setopt( curl, CURLOPT_USERAGENT, "libcurl-agent/1.0" ); /* some servers don't like requests that are made without a user-agent field, so we provide one */
        curl_easy_setopt( curl, CURLOPT_ERRORBUFFER, errbuf ); /* provide a buffer to store errors in */
        curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, "NLST" ); /* NLST or LIST */
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback ); /* send all data to this function */

        /*
         * Perform the custom curl request
         */
        res = curl_easy_perform( curl );
        if( res != CURLE_OK ) {
            cerr << "home-security: curl_easy_perform() failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
        } else {
            curl_easy_setopt( curl, CURLOPT_URL, remote_url );
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL ); /* reset write callback function */
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, NULL ); /* reset write data function */

            cout << "home-security: list size " << list.size << ", memory " << list.memory << endl;
            if( list.size > 0 ) {
                while( ( remote_file_name = strsep( &list.memory, delimiter ) ) != NULL ) {
                    if( strlen( remote_file_name ) > 0 ) {
                        /*
                         * First felete every file from the directory to be removed
                         */
                        cout << "home-security: Delete remote filename " << remote_file_name << endl;

                        remove_cmd.assign( "DELE " );
                        remove_cmd.append( remote_dir );
                        remove_cmd.append( "/" );
                        remove_cmd.append( remote_file_name );
                        curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, remove_cmd.c_str() );

                        /*
                         * Perform the custom curl request
                         */
                        res = curl_easy_perform(curl);
                        if( res != CURLE_OK ) {

                            /*
                             * Response code 250 after a DELETE command means the delete was succesful.
                             */
                            if( strcmp( "RETR response: 250", errbuf ) != 0 ) {
                                cerr << "home-security: " << remove_cmd << " failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
                            }
                        }
                    }
                }
            }

            /*
             * And finaly remove the directory itself
             */
            remove_cmd.assign( "RMD " );
            remove_cmd.append( remote_dir );
            curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, remove_cmd.c_str() );

            /*
             * Perform the custom curl request
             */
            res = curl_easy_perform(curl);
            if( res != CURLE_OK ) {

                /*
                 * Response code 250 after a RMD command means the delete was succesful.
                 * Response code 550 after a RMD command means the delete was not succesful.
                 * This happens sometimes when there were too many file in the directory to be removed in one go.
                 * Any next attempt will mostly be succesful.
                 */
                if( strcmp( "RETR response: 250", errbuf ) == 0 ) {
                    cout << "home-security: " << remove_cmd << " successful" << endl;
                } else if( strcmp( "RETR response: 550", errbuf ) == 0 ) {
                    cerr << "home-security: " << remove_cmd << " failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
                } else {
                    cerr << "home-security: " << remove_cmd << " failed: " << (int)res << ", " << curl_easy_strerror( res ) << endl;
                }
            }
        }
 
        /*
         * Always cleanup
         */
        curl_easy_cleanup( curl );
        free( list.memory );
    }

    /*
     * Always cleanup
     */
    curl_global_cleanup();

    return( (int)res );
}
