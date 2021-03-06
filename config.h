#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#include <map.h>

#define CONFIG_FILE_READ_OK 0
#define CONFIG_FILE_READ_ERROR -1
#define CONFIG_MISSING_KEY -2
#define CONFIG_INCONSISTENT_DATA -3

typedef struct {
	int port;
	const char *host;
	const char *web_root;
	const char *web_prefix;
	const char *api_prefix;
	const char *index_file;
	int buffer_size;
	char *buffer;
	const char *api_modules_path;
	int api_modules_number;
	const char **api_modules_names;
	map api_modules;
} s_config;

int get_server_config(s_config *c, char *config_path);

#endif
