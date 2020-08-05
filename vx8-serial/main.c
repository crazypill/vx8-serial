//
//  main.c
//  vx8-serial
//
//  Created by Alex Lelievre on 4/28/20.
//  Copyright © 2020 Far Out Labs. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>


#define PORT_ERROR -1



void buffer_input_flush()
{
    int c;
     // This will eat up all other characters
    while( (c = getchar()) != EOF && c != '\n' )
        ;
}


int main(int argc, const char * argv[])
{
    
    // open the /dev/cu.SLAB_USBtoUART
    
    bool blocking = false;

    //Settings structure old and new
    struct termios newtio;
    
    int fd = open( "/dev/rtsystem-0", O_RDWR | O_NOCTTY | (blocking ? 0 : O_NDELAY) );
    if( fd < 0 )
        return PORT_ERROR;

    bzero( &newtio, sizeof( newtio ) );
    
    if( cfsetispeed( &newtio, B9600 ) != 0 )
        return PORT_ERROR;
    if( cfsetospeed( &newtio, B9600 ) != 0 )
        return PORT_ERROR;
    
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;
    newtio.c_cflag &= ~CRTSCTS;
    
    // Hardware control of port
    newtio.c_cc[VTIME] = blocking ? 1 : 0; // Read-timout 100ms when blocking
    newtio.c_cc[VMIN] = 0;
    
    tcflush( fd, TCIFLUSH );
    
    // Acquire new port settings
    if( tcsetattr( fd, TCSANOW, &newtio ) != 0 )
        puts( strerror( errno ) );

    if( fd == -1 )
        return PORT_ERROR;

    char line_buffer[2048] = {0};
    ssize_t result = 0;
    
    while( 1 )
    {
        char* input = fgets( line_buffer, sizeof( line_buffer ), stdin );
        if( input )
        {
            if( strncmp( input, "done", strlen( "done" ) ) == 0 )
                return 0;
            
            if( input[0] != '\n' )
            {
                printf( "Sending: %s\n", input );
                ssize_t size = strlen( input );
                
                result = write( fd, input, size );
                if( result != size )
                    printf( "problem writing to serial.\n" );
                
                // write CR/LF?
                write( fd, "\r\n", 2 );
            }
            
            do
            {
                result = read( fd, line_buffer, sizeof( line_buffer ) );
                printf( "result: %ld\n", result );
                if( result > 0 )
                {
                    // skip first byte and null terminate
                    line_buffer[result] = '\0';
                    printf( "%s\n", &line_buffer[0] );
//                    result = 0;
                }
            }
            while( result > 0 );
        }
    }
    
    return 0;
}

// EOF
