/*--------------------------------------------------------------- 
 * Copyright (c) 2014                              
 * MUSE / Inria Paris-Roquencourt
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * MUSE team
 * Inria Paris-Roquencourt
 * http://www.inria.fr
 * ________________________________________________________________ 
 *
 * report_JSON.h
 * by Anna-Kaisa Pietilainen <anna-kaisa.pietilainen@inria.fr>
 *
 * ________________________________________________________________ */

#include "headers.h"
#include "Settings.hpp"
#include "util.h"
#include "Reporter.h"
#include "report_JSON.h"
#include "Locale.h"

void JSON_timestamp( char *timestamp, int length );
 
void JSON_stats( Transfer_Info *stats ) {
    max_size_t speed = (max_size_t)(((double)stats->TotalLen * 8.0) / (stats->endTime - stats->startTime));
    char timestamp[128];
    JSON_timestamp( timestamp, sizeof(timestamp) );
    if ( stats->mUDP != (char)kMode_Server ) {
        // TCP Reporting
        printf( reportJSON_bw_format, 
                timestamp, 
                (stats->reserved_delay == NULL ? "{}" : stats->reserved_delay),
                stats->transferID, 
                stats->startTime, 
                stats->endTime, 
                stats->TotalLen, 
                speed);
    } else {
        // UDP Reporting
        printf( reportJSON_bw_jitter_loss_format, 
                timestamp, 
                (stats->reserved_delay == NULL ? "{}" : stats->reserved_delay),
                stats->transferID, 
                stats->startTime, 
                stats->endTime, 
                stats->TotalLen, 
                speed,
                stats->jitter*1000.0, 
                stats->cntError, 
                stats->cntDatagrams,
                (100.0 * stats->cntError) / stats->cntDatagrams, stats->cntOutofOrder );
    }
    if ( stats->free == 1 && stats->reserved_delay != NULL ) {
        free( stats->reserved_delay );
    }
}

void *JSON_peer( Connection_Info *stats, int ID ) {
    
    // copy the inet_ntop into temp buffers, to avoid overwriting
    char local_addr[ REPORT_ADDRLEN ];
    char remote_addr[ REPORT_ADDRLEN ];
    char *buf = malloc( REPORT_ADDRLEN*2 + 10 );
    struct sockaddr *local = ((struct sockaddr*)&stats->local);
    struct sockaddr *peer = ((struct sockaddr*)&stats->peer);

    if ( local->sa_family == AF_INET ) {
        inet_ntop( AF_INET, &((struct sockaddr_in*)local)->sin_addr, 
                   local_addr, REPORT_ADDRLEN);
    }
#ifdef HAVE_IPV6
      else {
        inet_ntop( AF_INET6, &((struct sockaddr_in6*)local)->sin6_addr, 
                   local_addr, REPORT_ADDRLEN);
    }
#endif

    if ( peer->sa_family == AF_INET ) {
        inet_ntop( AF_INET, &((struct sockaddr_in*)peer)->sin_addr, 
                   remote_addr, REPORT_ADDRLEN);
    }
#ifdef HAVE_IPV6
      else {
        inet_ntop( AF_INET6, &((struct sockaddr_in6*)peer)->sin6_addr, 
                   remote_addr, REPORT_ADDRLEN);
    }
#endif

    snprintf(buf, REPORT_ADDRLEN*2+10, reportJSON_peer, 
             local_addr, ( local->sa_family == AF_INET ?
                          ntohs(((struct sockaddr_in*)local)->sin_port) :
#ifdef HAVE_IPV6
                          ntohs(((struct sockaddr_in6*)local)->sin6_port)),
#else
                          0),
#endif
            remote_addr, ( peer->sa_family == AF_INET ?
                          ntohs(((struct sockaddr_in*)peer)->sin_port) :
#ifdef HAVE_IPV6
                          ntohs(((struct sockaddr_in6*)peer)->sin6_port)));
#else
                          0));
#endif
    return buf;
}

void JSON_serverstats( Connection_Info *conn, Transfer_Info *stats ) {
    stats->reserved_delay = JSON_peer( conn, stats->transferID );
    stats->free = 1;
    JSON_stats( stats );
}

void JSON_timestamp( char *timestamp, int length ) {
    time_t times;
    struct tm *timest;
    times = time( NULL );
    timest = localtime( &times );
    strftime( timestamp, length,"{\"year\":%Y,\"month\":%m,\"day\":%d,\"hour\":%H,\"minute\":%M,\"second\":%S,\"ts\":%s}", timest );
}
