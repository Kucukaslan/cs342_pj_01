#include "shared_defs.h"
#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main()
{
    //  mq variables
    mqd_t mq_cli_s;
    struct ClientServerItem clientServerItem = {.interval_start = 0,
                                                .interval_width = 10,
                                                .interval_count = 10,
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
    // printf("mq_send success, item size = %d\n", (int) sizeof(struct ChildParentItem));
    // printf("clientServerItem->pid   = %d\n---------\n", clientServerItem.pid);
    // client sends the last message to the parent
    clientServerItem.done = SERVER_TERMINATE;
    int n = mq_send(mq_cli_s, (char *) &clientServerItem, sizeof(struct ChildParentItem), 0);
    if (n == -1) {
        perror("mq_send: termination notice failed\n");
    }
    // free the memory
    // close the sources mq and file
    mq_close(mq_cli_s);
    printf("mq_cli_s closed by client process, mq_cli_s id = %d\n", (int) mq_cli_s);
    /*
    mqd_t mq;
    struct mq_attr mq_attr;
    struct item *itemptr;
    int n;
    char *bufptr;
    int buflen;
    mq = mq_open(MQ_CLI_S, O_RDWR | O_CREAT, 0666, NULL);
    if (mq == -1) {
        perror("can not create msg queue\n");
        exit(1);
    }
    printf("mq created, mq id = %d\n", (int) mq);
    mq_getattr(mq, &mq_attr);

    printf("mq maximum msgsize = %d\n", (int) mq_attr.mq_msgsize);
    */
    /* allocate large enough space for the buffer to store
        an incoming message */
    /*
    buflen = mq_attr.mq_msgsize;
    bufptr = (char *) malloc(buflen);
    while (1) {
        n = mq_receive(mq, (char *) bufptr, buflen, NULL);
        if (n == -1) {
            perror("mq_receive failed\n");
            exit(1);
        }
        printf("mq_receive success, message size = %d\n", n);
        itemptr = (struct item *) bufptr;
        printf("received item->pid = %d\n", itemptr->pid);
        printf("\n");
    }
    free(bufptr);
    mq_close(mq);
     */
    return 0;
}
