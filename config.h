typedef struct {
	char *root;
	short port;
	char *host;
} s_config;

s_config *get_config();
