struct ServerClientItem {
    int interval;
    int interval_frequency;
    int status;
};

struct ClientServerItem {
    int done;
    int interval_start;
    int interval_width;
    int interval_count;
};
#define CHILD_CONTINUE 1
#define CHILD_TERMINATE 0
#define SERVER_CONTINUE 1
#define SERVER_TERMINATE 0
#define CLIENT_CONTINUE 0
#define CLIENT_TERMINATE 1

#define MQ_CLI_S "/21901779_b_CLIENT_SERVER"
#define MQ_S_CLI "/21901779_b_SERVER_CLIENT"