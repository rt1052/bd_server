#include "log.h"
#include "time.h"

LOG_DATA log_file;

/* create log file */
void log_write(char *str)
{
    FILE *fp = NULL;
    char buf[256];
    /* get system time */
    time_t time_v;
    time(&time_v);
    struct tm *time_p = localtime(&time_v);

    sprintf(log_file.path, "/var/log/backdoor/%4d-%02d.log", time_p->tm_year + 1900,
           time_p->tm_mon + 1);

    sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d    %s \r\n", time_p->tm_year + 1900,
          time_p->tm_mon + 1, time_p->tm_mday, time_p->tm_hour, time_p->tm_min,
          time_p->tm_sec, str);

    fp = fopen(log_file.path, "ab+");
    if (NULL != fp) {
        /* write log file */
        fwrite(buf, strlen(buf), 1, fp);
        /* file will not saved before closed */
        // fflush(fp);
        fclose(fp);
    }
}
