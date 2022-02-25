#include "shared_defs.h"
#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv) {
  //  mq variables
  // argv[1] intervalcount
  // argv[2] interval_width
  // argv[3] interval_start
  if (argc != 4 || !(atoi(argv[1]) >= 1 && atoi(argv[1]) <= 1000) ||
      !(atoi(argv[2]) >= 1 && atoi(argv[2]) <= 1000000)) {
    printf("Invalid parameters in histclient program\n");
    exit(-404);
  }
  int start = atoi(argv[3]);
  int width = atoi(argv[2]);
  int count = atoi(argv[1]);
  mqd_t mq_cli_s;
  struct ClientServerItem clientServerItem = {.interval_start = start,
                                              .interval_width = width,
                                              .interval_count = count,
                                              .done = SERVER_CONTINUE};
  // child opens mq queue STARTS
  mq_cli_s = mq_open(MQ_CLI_S, O_RDWR);
  if (mq_cli_s == -1) {
    printf("Client cannot open msg queue FOR ClientServer\n");
    exit(1);
  }
  printf("mq_cli_s opened by client process, mq_cli_s id = %d\n",
         (int)mq_cli_s);
  // create and send the first message
  {
    int n = mq_send(mq_cli_s, (char *)&clientServerItem,
                    sizeof(struct ClientServerItem), 0);
    if (n == -1) {
      perror("mq_send failed\n");
    }
  }
  // printf("mq_send success, item size = %d\n", (int) sizeof(struct
  // ChildParentItem)); printf("clientServerItem->pid   = %d\n---------\n",
  // clientServerItem.pid); client receive histogram data
  mqd_t mq_s_cli;
  struct mq_attr mq_s_cli_attr;
  // open/create m queue STARTS
  mq_s_cli = mq_open(MQ_S_CLI, O_RDWR);
  if (mq_s_cli == -1) {
    perror("Client cannot open msg queue FOR ServerClient\n");
    exit(1);
  }
  printf("mq_s_cli opened by client process, mq_s_cli id = %d\n",
         (int)mq_s_cli); // GET ATTR
  mq_getattr(mq_s_cli, &mq_s_cli_attr);
  printf("mq maximum msgsize = %d bytes\n", (int)mq_s_cli_attr.mq_msgsize);
  printf("mq maximum maxmsg = %d msgs\n", (int)mq_s_cli_attr.mq_maxmsg);

  int c_status = CLIENT_CONTINUE;
  int *freqs = (int *)malloc(count * sizeof(int));
  for (int i = 0; i < count; ++i) {
    freqs[i] = 0;
  }
  while (c_status == CLIENT_CONTINUE) {
    char *bufptr;
    int buflen;
    struct mq_attr mq_s_cli_attr;
    /* allocate large enough space for the buffer to store an incoming message
     */
    mq_getattr(mq_s_cli, &mq_s_cli_attr);
    buflen = mq_s_cli_attr.mq_msgsize;
    bufptr = (char *)malloc(buflen);
    int n;
    n = mq_receive(mq_s_cli, (char *)bufptr, buflen, NULL);
    if (n == -1) {
      perror("server to client mq_receive failed\n");
      free(bufptr);
      sleep(1);
      continue;
    } else {
      // printf("mq_receive success, message size = %d\n", n);
      struct ServerClientItem *itemptr = (struct ServerClientItem *)bufptr;
      if (itemptr->interval >= 0 && itemptr->interval_frequency >= 0) {
        freqs[itemptr->interval] = itemptr->interval_frequency;
      }
      c_status = itemptr->status;
      /*
      for (int i = 0; i < itemptr->size; ++i) {
        printf("[%d,%d):%d\n", start, start + width, itemptr->data[i]);
        start = start + width;
      }
       */
      free(bufptr);
      /**/
    }
  }
  for (int i = 0; i < count; ++i) {
    printf("[%d,%d):%d\n", start, start + width, freqs[i]);
    start = start + width;
  }
  free(freqs);
  mq_close(mq_s_cli);
  // client sends the last message to the parent
  clientServerItem.done = SERVER_TERMINATE;
  int n = mq_send(mq_cli_s, (char *)&clientServerItem,
                  sizeof(struct ClientServerItem), 0);
  if (n == -1) {
    perror("mq_send: termination notice failed\n");
  }
  printf("client to server termination msg sent\n");
  // close the mq
  mq_close(mq_cli_s);
  printf("mq_cli_s closed by client process, mq_cli_s id = %d\n",
         (int)mq_cli_s);
  return 0;
}
