#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "shared_defs.h"
void child(char *filename, int intervalcount, int intervalwidth, int intervalstart);
void processChildMQ(mqd_t mq);


int main(int argc, char **argv)
{
    // constants
    int N;
    // histclient <intervalcount> <intervalwidth> <intervalstart>
    // TODO
    int intervalcount = 10; // • Max value for <intervalcount> can be 1000. Min value is 1.
    int intervalwidth = 10; // • Max value for <intervalwidth> can be 1000000. Min value is 1.
    int intervalstart = 0;

    // ...
    if (argc < 2)
    {
        printf("There is no sufficient parameters in histserver program\n");
        exit(-404);
    }
    else
    {
        N = atoi(argv[1]);
    }

    // use argv directly to obtain file names

    pid_t *child_pids = (pid_t *)malloc(N * sizeof(pid_t)); // stores process id
    pid_t parentid;
    int x;

    parentid = getpid();

    // CREATE child mq
    mqd_t mq_c_s;
	struct mq_attr mq_c_s_attr;
	struct ChildParentItem *childParentItemPtr;
	int mq_c_s_n;
	char *bufptr;
	int buflen;

    // open/create m queue STARTS
    mq_c_s = mq_open(MQ_C_S, O_RDWR | O_CREAT, 0666, NULL);
    if (mq_c_s == -1) {
		perror("Cannot create msg queue FOR ChildParent\n");
		exit(1);
	}
    printf("mq_c_s created, mq_c_s id = %d\n", (int) mq_c_s);
    
    // GET ATTR
    mq_getattr(mq_c_s, &mq_c_s_attr);
  	printf("mq maximum msgsize = %d\n bytes", (int) mq_c_s_attr.mq_msgsize);

    /* allocate large enough space for the buffer to store 
        an incoming message */
    buflen = mq_c_s_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen);
    // open/create m queue ENDS


    printf("I am parent and my pid is: %d\n", parentid);
    int i;

    // creating childs.
    for (i = 0; i < N; ++i)
    {
        child_pids[i] = fork();
        if (child_pids[i] < 0)
        {
            printf("Cannot create %d'th child process.\nProgram terminated.", i);
            exit(child_pids[i]);
        }
        else if (child_pids[i] == 0)
        {
            int index = i + 2;
            /* this part executed by child process*/

            // struct ChildParentItem child(char* filename, int intervalcount, int intervalwidth, int intervalstart);
            child(argv[index], intervalcount, intervalwidth, intervalstart);
        }
        else
        {
            /* parent process */
            printf("parent created child %d and child pid= %d\n", i, child_pids[i]);
        }
    }

    while(1) {
        // processClientMQ();  
        processChildMQ(mq_c_s);   
        printf("\n--------------\n");     
        // sendClientMQ();
    }

    // wait for all children to terminate
    for (i = 0; i < N; ++i)
        wait(NULL);

    printf("all children terminated. bye... \n");

    /*
        mqd_t mq;
        struct item item;
        int n;

        mq = mq_open(MQ_CLI_S, O_RDWR);
        if (mq == -1) {
            perror("can not open msg queue\n");
            exit(1);
        }
        printf("mq opened, mq id = %d\n", (int) mq);
        int i = 0;

        while (1) {
            item.id = i;
            strcpy(item.astr, "this string is sample data\n");

            n = mq_send(mq, (char *) &item, sizeof(struct item), 0);

            if (n == -1) {
                perror("mq_send failed\n");
                exit(1);
            }

            printf("mq_send success, item size = %d\n",
                   (int) sizeof(struct item));
            printf("item->id   = %d\n", item.id);
            printf("item->astr = %s\n", item.astr);
            printf("\n");
            i++;
            sleep(1);
        }

        mq_close(mq);
        return 0;

        */
} // end of main method

void child(char *filename, int intervalcount, int intervalwidth, int intervalstart)
{
    printf("I am a child and mypid=%d\n", getpid());
    printf("Paremeters are %s, %d, %d, %d\n", filename, intervalcount, intervalwidth, intervalstart);

    mqd_t mq_c_s;
	struct mq_attr mq_c_s_attr;
	struct ChildParentItem childParentItemPtr;
	int mq_c_s_n;

    // child opens mq queue STARTS
    mq_c_s = mq_open(MQ_C_S, O_RDWR);
    if (mq_c_s == -1) {
		perror("Child cannot open msg queue FOR ChildParent\n");
		exit(1);
	}
    printf("mq_c_s opened, mq_c_s id = %d\n", (int) mq_c_s);
    
    int i = 0;

    // create and send the messages
    while(i < 5) {
        i++;
        childParentItemPtr.pid = getpid();

        int n = mq_send(mq_c_s, (char *) &childParentItemPtr, sizeof(struct ChildParentItem), 0);

        if (n == -1) {
            perror("mq_send failed\n");
            exit(1);
        }

        printf("mq_send success, item size = %d\n",
                (int) sizeof(struct ChildParentItem));
        printf("childParentItemPtr->pid   = %d\n", childParentItemPtr.pid);

    }


    exit(0); /* child is terminating */
}


void processChildMQ(mqd_t mq) {
    char *bufptr;
	int buflen;
    int n;
    n = mq_receive(mq, (char *) bufptr, buflen, NULL);
    if (n == -1) {
        perror("mq_receive failed\n");
        printf("mq_receivefailed = %d\n", n);
        return;
    }
    printf("mq_receive success, message size = %d\n", n);

    struct ChildParentItem *itemptr = (struct ChildParentItem *) bufptr;
    printf("itemptr->pid = %d", itemptr->pid);


}