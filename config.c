#include "config.h"
#include <libconfig.h>

char generated = 0;

int get_server_config(s_config *c)
{
	config_t cfg;

	config_init(&cfg);

	if (!config_read_file(&cfg, "config.cfg")) {
		config_destroy(&cfg);
		return CONFIG_FILE_READ_ERROR;
	}

	if (config_lookup_string(&cfg, "root", &((*c).root))
		&& config_lookup_int(&cfg, "port", &((*c).port))
		&& config_lookup_string(&cfg, "host", &((*c).host))
	) {
		generated = 1;
		return CONFIG_FILE_READ_OK;
	}
	else
		return CONFIG_MISSING_KEY;
}
