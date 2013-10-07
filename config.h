#define CONFIG_FILE_READ_OK 0
#define CONFIG_FILE_READ_ERROR -1
#define CONFIG_MISSING_KEY -2

typedef struct {
	const char *root;
	unsigned short port;
	const char *host;
} s_config;

int get_config(s_config *c);
