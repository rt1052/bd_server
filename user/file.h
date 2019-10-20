#ifndef _FILE_H_
#define _FILE_H_

#include "main.h"
#include "list.h"

typedef struct {
    int fd;
    bool active;
    char host[100];
    int port;

    int filesize;
    char filename[100];
    int percent;
    char direction[5];
} FILE_DATA;


extern LISTNODE *list_file, **list_file_P;









void *thread_file(void *arg);









#endif
