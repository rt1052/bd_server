#include "tcp.h"
#include "main.h"
#include "list.h"
#include "file.h"

int fd_tcp_cli;

int talkFlag;
bool getsFlag;
char getsBuf[1024];

LISTNODE *node_head, **node_head_p;

/* server recv data from client */
void *thread_recv(void *arg)
{
    char buf[4096];
    LISTNODE *node = (LISTNODE *)arg;
    CMD_DATA *data = (CMD_DATA *)node->data;
    int fd = data->fd;

    /* thread will auto free stack */
    pthread_detach(pthread_self());

    // fprintf(stderr, "thread recv %d \r\n", fd);
    while(data->active) {
        int len = recv(fd, buf, sizeof(buf), 0);  //MSG_WAITALL
        if (len > 0) {
            buf[len] = '\0';
            if (talkFlag == fd) {
                if (0 == strcmp(buf, "echo")) {  /* heartbeat packet */

                } else if (0 == strcmp(buf, "\r\r\n")) {  /* enter_key press */

                } else {
                    fprintf(stderr, "%s", buf);
                }
            }
        } else if (len == -1) {  /* timeout */
            usleep(10000);
        } else {  /* len == 0 client disconnected */
            data->active = false;
            break;
        }
    }
}

/* server send data to client */
void *thread_send(void *arg)
{
    LISTNODE *node = (LISTNODE *)arg;
    CMD_DATA *data = (CMD_DATA *)node->data;
    int fd = data->fd;
    char str[256];
    uint16_t cnt;

    /* thread will auto free stack */
    pthread_detach(pthread_self());
    
    sprintf(str, "%-17s %-7d connect", data->host, data->port);
    log_write(str);
    // fprintf(stderr, "thread send %d \r\n", fd);
    while(data->active) {
        if(talkFlag == fd) {
            if (true == getsFlag) {
                getsFlag = false;
                if (0 == strcmp(getsBuf, "exit")) {
                    talkFlag = 0;
                    fprintf(stderr, "\r\nuser# ");
                } else if (0 == strcmp(getsBuf, "test")) {
                    //send(fd, "\b", strlen("\b"), 0);
                } else if (0 == strcmp(getsBuf, "")) {
                    send(fd, "\r", strlen("\r"), 0);
                } else {
                    //fprintf(stderr, "recv:%s", buf);
                    send(fd, getsBuf, strlen(getsBuf), 0);
                }
            } else {
                cnt++;
                usleep(10000);
            }
        } else {
            cnt++;
            usleep(10000);
        }

        /* send heartbeat packet */
        if (cnt > 60 * 100) {
            cnt = 0;
            send(fd, "echo", strlen("echo"), 0);            
        }
    }

    usleep(100 * 1000);
    // fprintf(stderr, "socket %d closed \r\n", fd);
    close(fd);
    sprintf(str, "%-17s %-7d disconnect", data->host, data->port);
    log_write(str);    
    node_delete(node_head_p, node);
    talkFlag = 0;
}

void *thread_user(void *arg)
{
    char buf[1024];
    int num;

    fprintf(stderr, "user# ");
    while(1) {
        if(talkFlag == 0) {
            if (true == getsFlag) {
                getsFlag = false;

                if (0 == strcmp(getsBuf, "")) {
                    /* just press enter */
                } else if (0 == strncmp(getsBuf, "sel ", strlen("sel "))) {
                    sscanf(getsBuf+4, "%d", &num);
                    if (NULL != node_search_cmd(node_head, num)) {
                        fprintf(stderr, "\r\n********** %d ********** \r\n", num);
                        talkFlag = num;
                        continue;
                    } else {
                        fprintf(stderr, "conn %d not exist \r\n", num);
                    }
                } else if (0 == strcmp(getsBuf, "ls")) {
                    node_display_cmd(node_head);
                } else if (0 == strcmp(getsBuf, "lsl")) {
                    node_display_file(list_file);
                } else if (0 == strncmp(getsBuf, "del ", strlen("del "))) {
                    sscanf(getsBuf+4, "%d", &num);
                    LISTNODE *node = node_search_cmd(node_head, num);
                    if (NULL != node) {
                        CMD_DATA *data = (CMD_DATA *)node->data;
                        data->active = false;
                    } else {
                        fprintf(stderr, "conn %d not exist \r\n", num);
                    }
                } else if (0 == strncmp(getsBuf, "dell ", strlen("dell "))) {
                    sscanf(getsBuf+5, "%d", &num);
                    LISTNODE *node = node_search_cmd(list_file, num);
                    if (NULL != node) {
                        FILE_DATA *data = (FILE_DATA *)node->data;
                        data->active = false;
                    } else {
                        fprintf(stderr, "load %d not exist \r\n", num);
                    }
                } else if (0 == strcmp(getsBuf, "help")) {
                    fprintf(stderr, " -ls    list connected hosts \r\n");
                    fprintf(stderr, " -lsl   list file load hosts \r\n");
                    fprintf(stderr, " -sel   select one connection \r\n");
                    fprintf(stderr, " -del   delete one connection \r\n");
                    fprintf(stderr, " -dell  delete one file load process \r\n");
                    fprintf(stderr, " -help  this help \r\n");
                } else {
                    fprintf(stderr, "unknow command \r\n");
                }

                fprintf(stderr, "user# ");
            } else {
                usleep(1000);
            }
        } else {
            usleep(1000);
        }
    }
}

void *thread_gets(void *arg)
{
    while(1) {
        gets(getsBuf);
        getsFlag = true;
    }
}

void *thread_tcp(void *arg)
{
    int res;
    struct sockaddr_in server, client;

    pthread_t recv_thread, send_thread, user_thread, gets_thread;
    void *thread_result;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(TCP_PORT);

    /* create socket */
    int fd_tcp_srv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(fd_tcp_srv, SOL_SOCKET, SO_REUSEADDR, NULL, 1);
    bind(fd_tcp_srv, (struct sockaddr *)&server, sizeof(server));
    listen(fd_tcp_srv, 5);

    /* create node */
    node_head = node_create();
    node_head_p = &node_head;

    /* create thread */
    res = pthread_create(&user_thread, NULL, thread_user, NULL);
    if (res != 0) {
        fprintf(stderr, "user thread creation failed");
        exit(EXIT_FAILURE);
    }

    /* create thread */
    res = pthread_create(&gets_thread, NULL, thread_gets, NULL);
    if (res != 0) {
        fprintf(stderr, "gets thread creation failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        int len = sizeof(struct sockaddr);
        fd_tcp_cli = accept(fd_tcp_srv, (struct sockaddr *)&client, (socklen_t *)&len);

        struct timeval timeout = {0, 100 * 1000};
        setsockopt(fd_tcp_cli, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

        CMD_DATA *data = (CMD_DATA *)malloc(sizeof(CMD_DATA));
        data->fd = fd_tcp_cli;
        data->active = true;
        strcpy(data->host, inet_ntoa(client.sin_addr));
        data->port = ntohs(client.sin_port);

        node_insert(node_head_p, data);

        /* create thread */
        res = pthread_create(&recv_thread, NULL, thread_recv, node_head);
        if (res != 0) {
            fprintf(stderr, "recv thread creation failed");
            exit(EXIT_FAILURE);
        }

        /* create thread */
        res = pthread_create(&send_thread, NULL, thread_send, node_head);
        if (res != 0) {
            fprintf(stderr, "send thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    close(fd_tcp_srv);

    pthread_exit("tcp thread end");
}
