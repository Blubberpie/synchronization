/**
* server.c, copyright 2001 Steve Gribble
*
* The server is a single-threaded program.  First, it opens
* up a "listening socket" so that clients can connect to
* it.  Then, it enters a tight loop; in each iteration, it
* accepts a new connection from the client, reads a request,
* computes for a while, sends a response, then closes the
* connection.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>

#include "lib/socklib.h"
#include "common.h"
#include "threadpool.h"

extern int errno;

int   setup_listen(char *socketNumber);
char *read_request(int fd);
char *process_request(char *request, int *response_length);
void  send_response(int fd, char *response, int response_length);
void serve_request(void *arg);

int NUM_LOOPS;
int threads_in_pool;
int num_dispatches_called;

struct timeval prev_time;
struct timeval curr_time;

#define PRINT_EVERY 100

/**
* This program should be invoked as "./server <socketnumber>", for
* example, "./server 4342".
*/

int main(int argc, char **argv)
{
    gettimeofday(&prev_time, NULL);

    char buf[1000];
    int  socket_listen;
    int  socket_talk;
    int  dummy, len;

    if (argc != 4)
    {
        fprintf(stderr, "(SERVER): Invoke as  './server socknum [# threads_in_pool] [NUM_LOOPS]'\n");
        fprintf(stderr, "(SERVER): for example, './server 4434 2 100'\n");
        exit(-1);
    }

    /*
    * Set up the 'listening socket'.  This establishes a network
    * IP_address:port_number that other programs can connect with.
    */
    socket_listen = setup_listen(argv[1]);

    threads_in_pool = strtol(argv[2], NULL, 10);
    NUM_LOOPS = strtol(argv[3], NULL, 10);

    threadpool tp = create_threadpool(threads_in_pool);
    num_dispatches_called = 0;

    /*
    * Here's the main loop of our program.  Inside the loop, the
    * one thread in the server performs the following steps:
    *
    *  1) Wait on the socket for a new connection to arrive.  This
    *     is done using the "accept" library call.  The return value
    *     of "accept" is a file descriptor for a new data socket associated
    *     with the new connection.  The 'listening socket' still exists,
    *     so more connections can be made to it later.
    *
    *  2) Read a request off of the listening socket.  Requests
    *     are, by definition, REQUEST_SIZE bytes long.
    *
    *  3) Process the request.
    *
    *  4) Write a response back to the client.
    *
    *  5) Close the data socket associated with the connection
    */

    while(1) {
        socket_talk = saccept(socket_listen);  // step 1
        if (socket_talk < 0) {
            fprintf(stderr, "An error occurred in the server; a connection\n");
            fprintf(stderr, "failed because of ");
            perror("");
            exit(1);
        }
        dispatch(tp, serve_request, (void *) socket_talk);
    }
}

// Critical section
void serve_request(void *arg){

    num_dispatches_called++;
    if(num_dispatches_called == PRINT_EVERY){
        struct timeval duration;
        gettimeofday(&curr_time, NULL);
        timersub(&curr_time, &prev_time, &duration);
        printf("%lf\n", PRINT_EVERY/(duration.tv_sec + (duration.tv_usec / 1000000.0 )));
        prev_time = curr_time;
        num_dispatches_called = 0;
    }

    char *request = NULL;
    char *response = NULL;

    int socket_talk = (int) arg;

    request = read_request(socket_talk);  // step 2
    if (request != NULL) {
        int response_length;

        response = process_request(request, &response_length);  // step 3
        if (response != NULL) {
            send_response(socket_talk, response, response_length);  // step 4
        }
    }
    close(socket_talk);  // step 5

    // clean up allocated memory, if any
    if (request != NULL)
        free(request);
    if (response != NULL)
        free(response);
}

/**
* This function accepts a string of the form "5654", and opens up
* a listening socket on the port associated with that string.  In
* case of error, this function simply bonks out.
*/

int setup_listen(char *socketNumber) {
    int socket_listen;

    if ((socket_listen = slisten(socketNumber)) < 0) {
        perror("(SERVER): slisten");
        exit(1);
    }

    return socket_listen;
}

/**
* This function reads a request off of the given socket.
* This function is thread-safe.
*/

char *read_request(int fd) {
    char *request = (char *) malloc(REQUEST_SIZE*sizeof(char));
    int   ret;

    if (request == NULL) {
        fprintf(stderr, "(SERVER): out of memory!\n");
        exit(-1);
    }

    ret = correct_read(fd, request, REQUEST_SIZE);
    if (ret != REQUEST_SIZE) {
        free(request);
        request = NULL;
    }
    return request;
}

/**
* This function crunches on a request, returning a response.
* This is where all of the hard work happens.
* This function is thread-safe.
*/

char *process_request(char *request, int *response_length) {
    char *response = (char *) malloc(RESPONSE_SIZE*sizeof(char));
    int   i,j;

    // just do some mindless character munging here

    for (i=0; i<RESPONSE_SIZE; i++)
        response[i] = request[i%REQUEST_SIZE];

    for (j=0; j<NUM_LOOPS; j++) {
        for (i=0; i<RESPONSE_SIZE; i++) {
            char swap;

            swap = response[((i+1)%RESPONSE_SIZE)];
            response[((i+1)%RESPONSE_SIZE)] = response[i];
            response[i] = swap;
        }
    }
    *response_length = RESPONSE_SIZE;
    return response;
}
