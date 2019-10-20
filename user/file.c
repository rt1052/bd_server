#include "file.h"
#include "tcp.h"
#include "list.h"

LISTNODE *list_file, **list_file_P;

#define TCP_MAX_LEN 1400  // 1460

void *thread_load(void *arg)
{
    char buf[1024];
    char *file_buf = (char *)malloc(TCP_MAX_LEN);

    LISTNODE *node = (LISTNODE *)arg;
    FILE_DATA *data = (FILE_DATA *)node->data;
    int fd = data->fd;
    FILE *fp = NULL;
    int len;
    char str[256];
    uint64_t cnt;

    send(fd, "direction", strlen("direction"), 0);
    len = recv(fd, buf, sizeof(buf), 0);  //MSG_WAITALL
    buf[len] = '\0';
    strcpy(data->direction, buf);

    /* get file from client */
    if (0 == strcmp(data->direction, "in")) {
        /* get file information */
        send(fd, "info", strlen("info"), 0);
        len = recv(fd, buf, sizeof(buf), 0);  //MSG_WAITALL
        if (len > 0) {
            buf[len] = '\0';
            sscanf(buf, "%s %d", data->filename, &data->filesize);
        }
        /* create file */
        sprintf(str, "./download/%s", data->filename);
        fp = fopen(str, "wb");
        if (fp != NULL) {
            cnt = 0;
            while(data->active) {
                send(fd, "data", strlen("data"), 0);
                len = recv(fd, file_buf, TCP_MAX_LEN, 0);
                if (len > 0) {
                    fwrite(file_buf, len, 1, fp);
                    if (len < TCP_MAX_LEN) {
                        fprintf(stderr, "##len = %d", len);
                        break;
                    }
                    cnt += len;
                    data->percent = (cnt * 100) / data->filesize;
                } else {
                    break;
                }
            }
            fclose(fp);
        } else {
            fprintf(stderr, "create %s error \r\n", data->filename);
        }
      /* send file to client */
    } else if (0 == strcmp(data->direction, "out")) {
        /* get file information */
        send(fd, "info", strlen("info"), 0);
        len = recv(fd, buf, sizeof(buf), 0);  //MSG_WAITALL
        buf[len] = '\0';
        sscanf(buf, "%s", data->filename);

        /* upload file only store in the "upload" dir */
        sprintf(str, "./upload/%s", data->filename);

        fp = fopen(str, "rb");
        if (fp != NULL) {
            /* get file size */
            fseek(fp, 0L, SEEK_END);
            data->filesize = ftell(fp);
            //printf("fileSize = %d \r\n", fileSize);
            fseek(fp, 0L, SEEK_SET);

            cnt = 0;
            send(fd, "start", strlen("start"), 0);            
            while(data->active) {
                len = fread(file_buf, 1, TCP_MAX_LEN, fp);
                send(fd, file_buf, len, 0);
                cnt += len;
                data->percent = (cnt * 100) / data->filesize;                    
                if (len == TCP_MAX_LEN) {
                    len = recv(fd, buf, sizeof(buf), 0);
                    if (len <= 0) {
                        break;
                    }
                } else {
                    break;
                }
            }
            fclose(fp);
        } else {
            fprintf(stderr, "open %s error \r\n", data->filename);
        }
    } else {
        fprintf(stderr, "load direction %s error \r\n", data->direction);
    }

    free(file_buf);
    close(fd);
    node_delete(list_file_P, node);
}

void *thread_file(void *arg)
{
    pthread_t load_thread, upload_thread;
    struct sockaddr_in server, client;
    void *thread_result;
    int res;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(TCP_FILE_PORT);

    /* create socket */
    int fd_srv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(fd_srv, SOL_SOCKET, SO_REUSEADDR, NULL, 1);
    bind(fd_srv, (struct sockaddr *)&server, sizeof(server));
    listen(fd_srv, 5);

    list_file = node_create();
    list_file_P = &list_file;

    while(1)
    {
        int len = sizeof(struct sockaddr);
        int fd_cli = accept(fd_srv, (struct sockaddr *)&client, (socklen_t *)&len);

        FILE_DATA *data = (FILE_DATA *)malloc(sizeof(FILE_DATA));
        data->fd = fd_cli;
        data->active = true;
        strcpy(data->host, inet_ntoa(client.sin_addr));
        data->port = ntohs(client.sin_port);

        node_insert(list_file_P, data);

        /* create thread */
        int res = pthread_create(&load_thread, NULL, thread_load, list_file);
        if (res != 0) {
            printf("load thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
}
