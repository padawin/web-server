typedef struct {
	const char *root;
	unsigned short port;
	const char *host;
} s_config;

s_config *get_config();
