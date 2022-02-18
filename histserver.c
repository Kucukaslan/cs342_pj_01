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
    
    // open/create m queue STARTS
    mq_c_s = mq_open(MQ_C_S, O_RDWR | O_CREAT | O_NONBLOCK, 0666, NULL);
    if (mq_c_s == -1) {
		perror("Cannot create msg queue FOR ChildParent\n");
		exit(1);
	}
    printf("mq_c_s created, mq_c_s id = %d\n", (int) mq_c_s);
    
    // GET ATTR
    mq_getattr(mq_c_s, &mq_c_s_attr);
  	printf("mq maximum msgsize = %d\n bytes", (int) mq_c_s_attr.mq_msgsize);

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

    i = 0;
    while(i <100000) {
        // processClientMQ();  
        processChildMQ(mq_c_s);   
        //printf("%d:--------------\n\n", i);     
        // sendClientMQ();
        i++;
    }

    // wait for all children to terminate
    for (i = 0; i < N; ++i)
        wait(NULL);

    printf("all children terminated. bye... \n");


    mq_close(mq_c_s);

    free(child_pids);
    return 0;       
} // end of main method

/** opens and process the file
* opens the message queue
* sends the result of file processing to the parent via the message queue
* closes the message queue
* finally, it terminates the child process.
* @param filename the name of the file to be processed
* @param intervalcount the number of intervals to be created
* @param intervalwidth the width of each interval
* @param intervalstart the start value of the first interval
*/
void child(char *filename, int intervalcount, int intervalwidth, int intervalstart)
{
    printf("I am a child and mypid=%d\n", getpid());
    printf("Paremeters are %s, %d, %d, %d\n", filename, intervalcount, intervalwidth, intervalstart);
    int i = 0;

    // allocate memory for the array of intervals
    // a number is counted in an interval indexed i 
    // if number is in interval [i*intervalwidth, (i+1)*intervalwidth)
    int *intervals = (int *)malloc(intervalcount * sizeof(int));
    // initialize the array of intervals to 0 (better to be safe)
    for (i = 0; i < intervalcount; ++i)
        intervals[i] = 0;
    
    // try opening the file
    // int file = open(filename, O_RDONLY);
    FILE * file =  fopen (filename , "r");
    if (file == NULL ) {
            printf ("error in opening the file named %s\n", filename);
            exit (1);
    }

    // read the file
    // assuming it is   "an integer per line"
    int hasNext;
    char buf[1024];
    // BEWARE THAT fgets reads until the new line character
    // AND INCLUDES the new line character
    // but DOES NOT INCLUDE the end of file character.
    int number;
    int number_of_lines = 0;
    while(fgets (buf , 100 , file) != NULL) {
        //hasNext = read(file, buf, 1024);
        number = atoi(buf);
        // find the interval in which the number is located
        // and increment the corresponding interval
        int interval = (number - intervalstart) / intervalwidth;
        if (interval >= 0 && interval < intervalcount){
            intervals[interval]++;
            // TODO REMOVE DEBUG CODE 
            // printf("%d is in interval %d\n", number, interval);
        }
        else
            ;// todo uncomment? printf("number %d is not in interval [%d, %d)\n", number, intervalstart, intervalstart + intervalwidth * intervalcount);
        if (number_of_lines % 100 == 0)
        {
            printf("%d lines read\n", number_of_lines);
        }        
        number_of_lines++;
    }

    // print the frequency of each interval
    // with the interval start and end values
    for (i = 0; i < intervalcount; ++i)
        printf("interval [%d, %d) has %d numbers\n", intervalstart + i * intervalwidth, intervalstart + (i + 1) * intervalwidth, intervals[i]);

    //  mq variables
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
    printf("mq_c_s opened by child process, mq_c_s id = %d\n", (int) mq_c_s);
    

    // create and send the messages
    i=0;
    while(i < intervalcount) {
        childParentItemPtr.pid = getpid();
        childParentItemPtr.interval = i;
        childParentItemPtr.interval_frequency = intervals[i];

        int n = mq_send(mq_c_s, (char *) &childParentItemPtr, sizeof(struct ChildParentItem), 0);

        if (n == -1) {
            perror("mq_send failed\n");
        }

        printf("mq_send success, item size = %d\n",
                (int) sizeof(struct ChildParentItem));
        printf("childParentItemPtr->pid   = %d\n---------\n", childParentItemPtr.pid);
                
        i++;
    }

    free(intervals);
    mq_close(mq_c_s);
    printf("mq_c_s closed by child process, mq_c_s id = %d\n", (int) mq_c_s);

    exit(0); /* child is terminating */
}


void processChildMQ(mqd_t mq) {
    char *bufptr;
	int buflen;
	struct mq_attr mq_c_s_attr;

    /* allocate large enough space for the buffer to store 
        an incoming message */
    mq_getattr(mq, &mq_c_s_attr);
    buflen = mq_c_s_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen);

    int n;
    n = mq_receive(mq, (char *) bufptr, buflen, NULL);
    if (n == -1) {
//        perror("mq_receive failed\n");
    }
    else {
        printf("mq_receive success, message size = %d\n", n);

        struct ChildParentItem *itemptr = (struct ChildParentItem *) bufptr;
        printf("itemptr->pid = %d\n", itemptr->pid);
        printf("itemptr->interval = %d\n", itemptr->interval);
        printf("itemptr->interval_frequency = %d\n", itemptr->interval_frequency);
        printf("-----------------endMessageReceive\n");
    }
    free(bufptr);
}