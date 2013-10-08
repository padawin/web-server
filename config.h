typedef struct {
	const char *root;
	unsigned short port;
	const char *host;
} s_config;

int get_config(s_config *c);
