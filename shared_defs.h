struct item {
	int id;
	char astr[64];
};

struct ChildParentItem {
	int pid;
	int interval;
	int interval_frequency;
	int freq[1000]; 
	int status;
};

#define CHILD_CONTINUES 1
#define CHILD_TERMINATED 0
#define MQ_CLI_S "/21901779_a_CLIENT_SERVER"
#define MQ_S_C "/21901779_a_SERVER_CLIENT"
#define MQ_C_S "/21901779_a_CHILD_SERVER"