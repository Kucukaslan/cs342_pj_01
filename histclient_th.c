#include "shared_defs_th.h"
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    //  mq variables
    if (argc != 4 || !(atoi(argv[1]) >= 1 && atoi(argv[1]) <= 1000) ||
        !(atoi(argv[2]) >= 1 && atoi(argv[2]) <= 1000000)) {
        printf("Invalid parameters in histclient program\n");
        exit(-404);
    }
    mqd_t mq_cli_s;
    int interval_start = atoi(argv[3]);
    int interval_count = atoi(argv[1]);
    int interval_width = atoi(argv[2]);
    struct ClientServerItem clientServerItem = {.interval_start = interval_start,
                                                .interval_width = interval_width,
                                                .interval_count = interval_count,
                                                .done = SERVER_CONTINUE};
    // child opens mq queue STARTS
    mq_cli_s = mq_open(MQ_CLI_S, O_RDWR);
    if (mq_cli_s == -1) {
        printf("Client cannot open msg queue FOR ClientServer\n");
        exit(1);
    }
    printf("mq_cli_s opened by client process, mq_cli_s id = %d\n", (int) mq_cli_s);
    // create and send the first message
    {
        int n = mq_send(mq_cli_s, (char *) &clientServerItem, sizeof(struct ClientServerItem), 0);
        if (n == -1) {
            perror("mq_send failed\n");
        }
    }
    
    // client receive histogram data
    mqd_t mq_s_cli;
    struct mq_attr mq_s_cli_attr;
    // open/create m queue STARTS
    mq_s_cli = mq_open(MQ_S_CLI, O_RDWR);
    if (mq_s_cli == -1) {
        perror("Client cannot open msg queue FOR ServerClient\n");
        exit(1);
    }
    printf("mq_s_cli opened by client process, mq_s_cli id = %d\n", (int) mq_s_cli);
    
    // GET ATTR
    mq_getattr(mq_s_cli, &mq_s_cli_attr);
    printf("mq maximum msgsize = %d\n bytes", (int) mq_s_cli_attr.mq_msgsize);

    // prepare to receive histogram data
    int *histogram = (int *) malloc(sizeof(int) * interval_count); 
    char *bufptr;
    int buflen;
    buflen = mq_s_cli_attr.mq_msgsize;
    bufptr = (char *) malloc(buflen);
    int n;
    int status = CLIENT_CONTINUE;
    while (status == CLIENT_CONTINUE) {
        //printf("Client is waiting for histogram data\n");
        n = mq_receive(mq_s_cli, (char *) bufptr, buflen, NULL);
        if (n == -1) {
            //printf("server to client mq_receive failed\n");
            // sleep(1);
            continue;
        } else {
            struct ServerClientItem *itemptr = (struct ServerClientItem *) bufptr;
            //printf("mq_receive success, id: %d; status: %d\n", itemptr->interval, itemptr->status);
            status = itemptr->status;
            if (itemptr-> interval >= 0 && itemptr-> interval < interval_count) {
                histogram[itemptr->interval] = itemptr->interval_frequency;
            }
            else {
                printf("Invalid interval received from server. index: %d; interval: %d; interval_freq: %d \n", itemptr->interval, interval_start + interval_width*itemptr->interval, itemptr->interval_frequency); 
            }
        }
    }
    free(bufptr);
    mq_close(mq_s_cli);

    // print the histogram
    int tmp = interval_start;
    for (int i = 0; i < interval_count; i++) {
        tmp = interval_start + interval_width*i;
        printf("[%d: %d):%d\n", tmp, tmp + interval_width, histogram[i]);
    }

    // client sends the last message to the parent
    clientServerItem.done = SERVER_TERMINATE;
    n = mq_send(mq_cli_s, (char *) &clientServerItem, sizeof(struct ClientServerItem), 0);
    if (n == -1) {
        perror("mq_send: termination notice failed\n");
    }
    printf("client to server termination msg sent\n");
    // close the mq
    mq_close(mq_cli_s);
    printf("mq_cli_s closed by client process, mq_cli_s id = %d\n", (int) mq_cli_s);

    free(histogram);
    return 0;
}
