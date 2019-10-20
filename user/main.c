#include "main.h"
#include "file.h"
#include "tcp.h"

int msg_cmd;

int main(int argc, char **argv)
{
    int ch;
    pthread_t tcp_thread, file_thread;
    void *thread_result;

    printf("\r\n************** welcome *************** \r\n");
    while ((ch = getopt(argc, argv, "s:h")) != -1) {
        switch (ch) {
            case 's':
                // printf("The argument of -l is %s\n\n", optarg);
                //sscanf(optarg, "%d", &min_sz);
                break;
            case 'h':
            case '?':
                printf("\r\nUsage: network [options] \r\n");
                printf("    -h  this help \n\r");
                printf("    -s  set minimum buf size \r\n");
                printf("\r\n");
                return -1;
        }
    }

    log_write("reboot");

    /* create msg queue */
    msg_cmd = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (msg_cmd == -1) {
        printf("create msg queue error \r\n");
        exit(EXIT_FAILURE);
    }

    /* create thread */
    int res = pthread_create(&tcp_thread, NULL, thread_tcp, NULL);
    if (res != 0) {
        printf("tcp thread creation failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&file_thread, NULL, thread_file, NULL);
    if (res != 0) {
        printf("file thread creation failed");
        exit(EXIT_FAILURE);
    }

    /* wait for thread finish */
    res = pthread_join(file_thread, &thread_result);
    if (res != 0) {
        printf("file thread join failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_join(tcp_thread, &thread_result);
    if (res != 0) {
        printf("tcp thread join failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
