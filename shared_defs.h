struct item {
	int id;
	char astr[64];
};

struct ChildParentItem {
	int pid;
};

#define MQ_CLI_S "/21901779_a_CLIENT_SERVER"
#define MQ_S_C "/21901779_a_SERVER_CLIENT"
#define MQ_C_S "/21901779_a_CHILD_SERVER"