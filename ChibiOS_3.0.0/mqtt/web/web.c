/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * This file is a modified version of the lwIP web server demo. The original
 * author is unknown because the file didn't contain any license information.
 */

/**
 * @file web.c
 * @brief HTTP server wrapper thread code.
 * @addtogroup WEB_THREAD
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "web.h"

#if LWIP_NETCONN

static const char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_index_html[] = "<html><head><title>Congrats!</title></head><body><h1>Welcome to our lwIP HTTP server!</h1><p>This is a small test page.</body></html>";

static void http_server_serve( struct netconn* conn )
{
    struct netbuf* inbuf;
    char* buf;
    u16_t buflen;
    err_t err;

    /* Read the data from the port, blocking if nothing yet there.
     We assume the request (the part we care about) is in one netbuf */
//  err = netconn_recv(conn, &inbuf);
    inbuf = netconn_recv( conn );
    err = ERR_OK;

    if( err == ERR_OK )
    {
        netbuf_data( inbuf, ( void** )&buf, &buflen );

        /* Is this an HTTP GET command? (only check the first 5 chars, since
        there are other formats for GET, and we're keeping it very simple )*/
        if( buflen >= 5 &&
                buf[0] == 'G' &&
                buf[1] == 'E' &&
                buf[2] == 'T' &&
                buf[3] == ' ' &&
                buf[4] == '/' )
        {

            /* Send the HTML header
                   * subtract 1 from the size, since we dont send the \0 in the string
                   * NETCONN_NOCOPY: our data is const static, so no need to copy it
             */
            netconn_write( conn, http_html_hdr, sizeof( http_html_hdr ) - 1, NETCONN_NOCOPY );

            /* Send our HTML page */
            netconn_write( conn, http_index_html, sizeof( http_index_html ) - 1, NETCONN_NOCOPY );
        }
    }
    /* Close the connection (server closes in HTTP) */
    netconn_close( conn );

    /* Delete the buffer (netconn_recv gives us ownership,
     so we have to make sure to deallocate the buffer) */
    netbuf_delete( inbuf );
}

/**
 * Stack area for the http thread.
 */
THD_WORKING_AREA( wa_http_server, WEB_THREAD_STACK_SIZE );

/**
 * HTTP server thread.
 */
msg_t http_server( void* p )
{
    struct netconn* conn, *newconn;
    err_t err;

    ( void )p;

    /* Create a new TCP connection handle */
    conn = netconn_new( NETCONN_TCP );
    LWIP_ERROR( "http_server: invalid conn", ( conn != NULL ), return MSG_RESET; );

    chprintf( ( void* ) &SD6, "Running http server\r\n" );

    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind( conn, NULL, WEB_THREAD_PORT );

    /* Put the connection into LISTEN state */
    netconn_listen( conn );

    /* Goes to the final priority after initialization.*/
    chThdSetPriority( WEB_THREAD_PRIORITY );

    while( 1 )
    {
        chprintf( ( void* ) &SD6, "Waiting for connection\r\n" );
        //err = netconn_accept( conn, &newconn );
        newconn = netconn_accept( conn );
        err = ERR_OK;
        
        if( err != ERR_OK )
        { continue; }
        http_server_serve( newconn );
        netconn_delete( newconn );
    }
    return MSG_OK;
}

#endif /* LWIP_NETCONN */

/** @} */
