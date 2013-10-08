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

	if (config_lookup_int(&cfg, "port", &(c->port))
		&& config_lookup_string(&cfg, "host", &(c->host))
		&& config_lookup_string(&cfg, "web_root", &(c->web_root))
		&& config_lookup_string(&cfg, "web_prefix", &(c->web_prefix))
		&& config_lookup_string(&cfg, "api_prefix", &(c->api_prefix))
		&& config_lookup_string(&cfg, "index_file", &(c->index_file))
	) {
		generated = 1;
		return CONFIG_FILE_READ_OK;
	}
	else
		return CONFIG_MISSING_KEY;
}
