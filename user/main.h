#ifndef _MAIN_H_
#define _MAIN_H_

#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <stdarg.h> 
#include <string.h> 
#include <sys/msg.h>
#include <stdbool.h>



typedef struct {
    long int type;
    void *pdata;
} msg_st;



extern int msg_cmd;
extern int min_sz;










#endif


