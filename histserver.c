#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "shared_defs.h"

void child(char* filename, int intervalcount, int intervalwidth, int intervalstart) {
            printf("I am a child and mypid=%d\n", getpid());
            printf("Paremeters are %s, %d, %d, %d\n", filename, intervalcount,  intervalwidth,  intervalstart );
            exit(0); /* child is terminating */

}
int main(int argc, char **argv)
{
    // constants
    int N;
    // histclient <intervalcount> <intervalwidth> <intervalstart>
    const int intervalcount; // • Max value for <intervalcount> can be 1000. Min value is 1. 
    const int intervalwidth; // • Max value for <intervalwidth> can be 1000000. Min value is 1. 
    const int intervalstart;
    
     
    // ...
    if(argc < 2 ) {
        printf("There is no sufficient parameters in histserver program\n");
        exit(-404);
    }
    else{
        N = atoi(argv[1]);
    }

    // use argv directly to obtain file names

    pid_t* child_pids = (pid_t* ) malloc(N* sizeof(pid_t)); // stores process id
    pid_t parentid;
    int x;
    
    parentid = getpid();

    printf("I am parent and my pid is: %d\n", parentid);
    int i;
    for (i = 0; i < N; ++i)
    {
        child_pids[i] = fork();
        if( child_pids[i] < 0 ) {
            printf("Cannot create %d'th child process.\nProgram terminated.", i);
            exit(child_pids[i]);
        }
        else if (child_pids[i] == 0)
        {
            int index = i + 2;
            /* this part executed by child process*/

            // struct ChildParentItem child(char* filename, int intervalcount, int intervalwidth, int intervalstart);
            child(argv[index],  intervalcount,  intervalwidth,  intervalstart);

        }
        else
        {
            /* parent process */
            printf("parent created child %d and child pid= %d\n", i, child_pids[i]);
        }
    }

    // wait for all children to terminate
    for (i = 0; i < 3; ++i)
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

 