#include "shared_defs_th.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
typedef struct targ targ;// some weird name collusion, idk
struct targ {
    char *filename;
    int intervalcount;
    int intervalwidth;
    int intervalstart;
    int shared_arr_index;
};
void child_thread(struct targ *arg);

int processChildMQ(mqd_t mq, int *interval_freq);
int **shared_arr;
int main(int argc, char **argv)
{
    // constants
    int N = 0;
    // histclient <intervalcount> <intervalwidth> <intervalstart>

    int intervalcount = 10;// • Max value for <intervalcount> can be 1000. Min value is 1.
    int intervalwidth = 10;// • Max value for <intervalwidth> can be 1000000. Min value is 1.
    int intervalstart = 0;
    int s_status = SERVER_CONTINUE;
    int *interval_frequencies;
    // ...
    if (argc < 2) {
        printf("There are not sufficient parameters in histserver program\n");
        exit(-404);
    } else {
        N = atoi(argv[1]);
    }
    // use argv directly to obtain file names
    pthread_t *child_thread_ids = (pthread_t *) malloc(N * sizeof(pthread_t));// stores process id
    struct targ *thread_args = (targ *) malloc(N * sizeof(targ));
    pid_t parentid;
    parentid = getpid();
    // CREATE server to client mq
    mqd_t mq_s_cli;
    struct mq_attr mq_s_cli_attr;
    // open/create m queue STARTS
    mq_s_cli = mq_open(MQ_S_CLI, O_RDWR | O_CREAT | O_NONBLOCK, 0666, NULL);
    if (mq_s_cli == -1) {
        perror("Cannot create msg queue FOR ServerClient\n");
        exit(1);
    }
    printf("mq_s_cli created, mq_s_cli id = %d\n", (int) mq_s_cli);
    // GET ATTR
    mq_getattr(mq_s_cli, &mq_s_cli_attr);
    printf("mq maximum msgsize = %d\n bytes", (int) mq_s_cli_attr.mq_msgsize);
    // open/create m queue ENDS
    // CREATE client to server mq
    mqd_t mq_cli_s;
    struct mq_attr mq_cli_s_attr;
    // open/create m queue STARTS
    mq_cli_s = mq_open(MQ_CLI_S, O_RDWR | O_CREAT | O_NONBLOCK, 0666, NULL);
    if (mq_cli_s == -1) {
        perror("Cannot create msg queue FOR ClientServer\n");
        exit(1);
    }
    printf("mq_cli_s created, mq_cli_s id = %d\n", (int) mq_cli_s);
    // GET ATTR
    mq_getattr(mq_cli_s, &mq_cli_s_attr);
    printf("mq maximum msgsize = %d\n bytes\n", (int) mq_cli_s_attr.mq_msgsize);
    // obtain intervalcount, intervalwidth, intervalstart from client
    while (1) {
        char *bufptr;
        int buflen;
        struct mq_attr mq_cli_s_attr;
        /* allocate large enough space for the buffer to store an incoming message */
        mq_getattr(mq_cli_s, &mq_cli_s_attr);
        buflen = mq_cli_s_attr.mq_msgsize;
        bufptr = (char *) malloc(buflen);
        int n;
        n = mq_receive(mq_cli_s, (char *) bufptr, buflen, NULL);
        if (n == -1) {
            perror("mq_receive failed\n");
            free(bufptr);
            sleep(1);
            continue;
        } else {
            // printf("mq_receive success, message size = %d\n", n);
            struct ClientServerItem *itemptr = (struct ClientServerItem *) bufptr;
            s_status = itemptr->done;
            intervalwidth = itemptr->interval_width;
            intervalstart = itemptr->interval_start;
            intervalcount = itemptr->interval_count;
            printf("s_status = %d\n", s_status);
            printf("intervalwidth = %d\n", intervalwidth);
            printf("intervalstart = %d\n", intervalstart);
            printf("intervalcount = %d\n", intervalcount);
            printf("-----------------endMessageReceive\n");
            free(bufptr);
            break;
        }
        /*
        struct ClientServerItem params = processClientMQ(mq_cli_s);
        sleep(1);
        */
        break;
    }
    printf("I am parent and my pid is: %d\n", parentid);
    int i;
    // fill the 1st dim of shared_arr
    shared_arr = (int **) malloc(N * sizeof(int *));
    for (int j = 0; j < N; ++j) {
        shared_arr[j] = malloc(intervalcount * sizeof(int));
    }
    // creating childs.
    for (int j = 0; j < N; ++j) {
        thread_args[j].intervalcount = intervalcount;
        thread_args[j].intervalstart = intervalstart;
        thread_args[j].intervalwidth = intervalwidth;
        thread_args[j].filename = argv[j + 2];
        thread_args[j].shared_arr_index = j;
        int err = pthread_create(&(child_thread_ids[j]), NULL, (void *(*) (void *) ) child_thread, (void *) &(thread_args[j]));
        if (child_thread_ids[j] < 0) {
            printf("Cannot create %d'th child thread with err=%d.\nProgram terminated.", j, err);
            free(child_thread_ids);
            // TODO free shared arr
            // TODO fix this on fork/exec as well
            exit(err);
        }
            printf("parent created child thread number %d and child thread id= %lu\n", j, child_thread_ids[j]);
    }
    // allocate memory for interval_frequencies
    interval_frequencies = (int *) malloc(intervalcount * sizeof(int));
    // set all interval_frequencies to 0
    for (int j = 0; j < intervalcount; j++) {
        interval_frequencies[j] = 0;
    }
    printf("main: waiting all threads to terminate\n");
    for (i = 0; i < N;) {
        //char *retmsg;
        int ret;
        ret = pthread_join(child_thread_ids[i], NULL);
        if (ret != 0) {
            printf("thread join failed \n");
            printf("err ret: %d\n%lu\n", ret, child_thread_ids[i]);
            //printf("err retmsg: %s\n", retmsg);
            sleep(1);
        } else {
            i++;
            printf("thread terminated\n");
            // we got the reason as the string pointed by retmsg
            // space for that was allocated in thread function; now freeing.
            //free(retmsg);
        }
    }
    printf("main: all threads terminated. Calculating sums.\n");
    /*
    int terminated_threads = 0;
    while (terminated_threads < N) {
        if () {
            terminated_threads++;
        }
        //printf("%d:--------------\n\n", i);
        i++;
    }
    */
    // calculate final intervals
    for (int j = 0; j < intervalcount; ++j) {
        int single_interval_sum = 0;
        for (int k = 0; k < N; ++k) {
            single_interval_sum = single_interval_sum + shared_arr[k][j];
        }
        interval_frequencies[j] = single_interval_sum;
    }
    // print the start and end intervals result of interval_frequencies
    struct ServerClientItem serverClientItem;
    serverClientItem.size = intervalcount;
    for (int j = 0; j < intervalcount; j++) {
        serverClientItem.data[j] = interval_frequencies[j];
        printf("%d: %d\n", j, interval_frequencies[j]);
    }
    int n = mq_send(mq_s_cli, (char *) &serverClientItem, sizeof(struct ServerClientItem), 0);
    if (n == -1) {
        perror("mq_send: serverClientItem failed\n");
    }// print the address of interval_frequencies
    printf("interval_frequencies address: %p\n", interval_frequencies);
    // send the results to the client
    // sendClientMQ();
    // wait termination signal from client
    // waitClientMQ();
    // wait for all children to terminate
    for (i = 0; i < N; ++i)
        wait(NULL);
    printf("all children terminated.\n");

    while (s_status != SERVER_TERMINATE) {
        char *bufptr;
        int buflen;
        struct mq_attr mq_cli_s_attr;
        /* allocate large enough space for the buffer to store an incoming message */
        mq_getattr(mq_cli_s, &mq_cli_s_attr);
        buflen = mq_cli_s_attr.mq_msgsize;
        bufptr = (char *) malloc(buflen);
        int n;
        n = mq_receive(mq_cli_s, (char *) bufptr, buflen, NULL);
        if (n == -1) {
            perror("mq_receive failed, so still waiting for termination msg\n");
            free(bufptr);
            sleep(1);
            continue;
        } else {
            // printf("mq_receive success, message size = %d\n", n);
            struct ClientServerItem *itemptr = (struct ClientServerItem *) bufptr;
            s_status = itemptr->done;
            printf("s_status = %d\n", s_status);
            free(bufptr);
            break;
            /**/
        }
    }
    printf("termination msg received from client\n");
    // close and unlink the message queues
    mq_close(mq_s_cli);
    mq_close(mq_cli_s);

    mq_unlink(MQ_CLI_S);
    mq_unlink(MQ_S_CLI);
    // free the memory allocated
    free(child_thread_ids);
    printf("child_thread_ids freed\n");
    free(interval_frequencies);
    printf("interval_frequencies freed\n");
    for (int j = 0; j < N; ++j) {
        free(shared_arr[j]);
    }
    free(shared_arr);
    printf("shared_arr freed\n");
    free(thread_args);
    printf("thread_args freed\n");
    return 0;
}// end of main method
/** opens and process the file
* opens the message queue
* sends the result of file processing to the parent via the message queue
* closes the message queue
* finally, it terminates the thread.
* @param filename the name of the file to be processed
* @param intervalcount the number of intervals to be created
* @param intervalwidth the width of each interval
* @param intervalstart the start value of the first interval
*/
void  child_thread(struct targ *arg)
{
    char *filename    = arg->filename;
    int intervalcount = arg->intervalcount;
    int intervalwidth = arg->intervalwidth;
    int intervalstart = arg->intervalstart;
    int shared_arr_index =  arg->shared_arr_index;
    //printf("I am a child and my thread id=%d\n", getpid());
    printf("Parameters are %s, %d, %d, %d, %d\n", filename, intervalcount, intervalwidth, intervalstart,
           shared_arr_index);
    int i = 0;
    // allocate memory for the array of intervals
    // a number is counted in an interval indexed i
    // if number is in interval [i*intervalwidth, (i+1)*intervalwidth)
    //int *intervals = (int *) malloc(intervalcount * sizeof(int));
    //shared_arr[shared_arr_index] = intervals;
    // initialize the array of intervals to 0 (better to be safe)
    for (i = 0; i < intervalcount; ++i)
        shared_arr[shared_arr_index][i] = 0;
    // try opening the file
    // int file = open(filename, O_RDONLY);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("error in opening the file named %s\n", filename);
        exit(1);
    }
    // read the file
    // assuming it is   "an integer per line"
    //int hasNext;
    char buf[1024];
    // BEWARE THAT fgets reads until the new line character
    // AND INCLUDES the new line character
    // but DOES NOT INCLUDE the end of file character.
    int number;
    int number_of_lines = 0;
    while (fgets(buf, 100, file) != NULL) {
        //hasNext = read(file, buf, 1024);
        number = atoi(buf);
        // find the interval in which the number is located
        // and increment the corresponding interval
        int interval = (number - intervalstart) / intervalwidth;
        if (interval >= 0 && interval < intervalcount) {
            shared_arr[shared_arr_index][interval]++;
            // TODO REMOVE DEBUG CODE
            // printf("%d is in interval %d\n", number, interval);
        } else
            ;// todo uncomment? printf("number %d is not in interval [%d, %d)\n", number, intervalstart, intervalstart + intervalwidth * intervalcount);
        // if (number_of_lines % 100 == 0)
        // {
        //     printf("%d lines read\n", number_of_lines);
        // }
        number_of_lines++;
    }
    // print the frequency of each interval
    // with the interval start and end values
    for (i = 0; i < intervalcount; ++i)
        printf("interval [%d, %d) has %d numbers\n", intervalstart + i * intervalwidth,
               intervalstart + (i + 1) * intervalwidth, shared_arr[shared_arr_index][i]);
    //  mq variables
    /*
    mqd_t mq_c_s;
    struct ChildParentItem childParentItemPtr;
    //	int mq_c_s_n;
    // child opens mq queue STARTS
    mq_c_s = mq_open(MQ_C_S, O_RDWR);
    if (mq_c_s == -1) {
        perror("Child cannot open msg queue FOR ChildParent\n");
        exit(1);
    }
    printf("mq_c_s opened by child process, mq_c_s id = %d\n", (int) mq_c_s);
    // create and send the messages
    i = 0;
    childParentItemPtr.status = CHILD_CONTINUE;
    while (i < intervalcount) {
        childParentItemPtr.pid = getpid();
        childParentItemPtr.interval = i;
        childParentItemPtr.interval_frequency = intervals[i];
        int n = mq_send(mq_c_s, (char *) &childParentItemPtr, sizeof(struct ChildParentItem), 0);
        if (n == -1) {
            perror("mq_send failed\n");
        }
        // printf("mq_send success, item size = %d\n", (int) sizeof(struct ChildParentItem));
        // printf("childParentItemPtr->pid   = %d\n---------\n", childParentItemPtr.pid);
        i++;
    }
    // child sends the last message to the parent
    childParentItemPtr.status = CHILD_TERMINATE;
    childParentItemPtr.pid = getpid();
    childParentItemPtr.interval = -1;
    childParentItemPtr.interval_frequency = -1;
    int n = mq_send(mq_c_s, (char *) &childParentItemPtr, sizeof(struct ChildParentItem), 0);
    if (n == -1) {
        perror("mq_send: termination notice failed\n");
    }
     */
    // free the memory
    // free(intervals); // TODO freed in main
    // close and unlink (delete) the mq
    //mq_close(mq_c_s);
    // close the file
    fclose(file);
    pthread_exit(NULL);
    //printf("mq_c_s closed by child thread, mq_c_s id = %d\n", (int) mq_c_s);
}


// ########### END OF CHILD
// ########### START OF PROCESS CHILD MQ
int processChildMQ(mqd_t mq, int *interval_freq)
{
    char *bufptr;
    int buflen;
    struct mq_attr mq_c_s_attr;
    int status = CHILD_CONTINUE;
    /* allocate large enough space for the buffer to store
        an incoming message */
    mq_getattr(mq, &mq_c_s_attr);
    buflen = mq_c_s_attr.mq_msgsize;
    bufptr = (char *) malloc(buflen);
    int n;
    n = mq_receive(mq, (char *) bufptr, buflen, NULL);
    if (n == -1) {
        // perror("mq_receive failed\n");
    } else {
        // printf("mq_receive success, message size = %d\n", n);
        struct ChildParentItem *itemptr = (struct ChildParentItem *) bufptr;
        /*
            printf("itemptr->pid = %d\n", itemptr->pid);
            printf("itemptr->interval = %d\n", itemptr->interval);
            printf("itemptr->interval_frequency = %d\n", itemptr->interval_frequency);
            printf("-----------------endMessageReceive\n");
        */
        status = itemptr->status;
        interval_freq[itemptr->interval] = interval_freq[itemptr->interval] + itemptr->interval_frequency;
    }
    free(bufptr);
    return status;
}