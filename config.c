#include <stdlib.h>
#include "config.h"
#include <libconfig.h>

int get_server_config(s_config *c, char *config_path)
{
	config_t cfg;
	config_setting_t *modules_setting, *module_setting;
	unsigned int count, i;
	const char *module_name;

	config_init(&cfg);

	if (!config_read_file(&cfg, config_path)) {
		config_destroy(&cfg);
		return CONFIG_FILE_READ_ERROR;
	}

	if (config_lookup_int(&cfg, "port", &(c->port))
		&& config_lookup_string(&cfg, "host", &(c->host))
		&& config_lookup_string(&cfg, "web_root", &(c->web_root))
		&& config_lookup_string(&cfg, "web_prefix", &(c->web_prefix))
		&& config_lookup_string(&cfg, "api_prefix", &(c->api_prefix))
		&& config_lookup_string(&cfg, "index_file", &(c->index_file))
		&& config_lookup_string(&cfg, "api_modules_path", &(c->api_modules_path))
		&& config_lookup_int(&cfg, "buffer_size", &(c->buffer_size))
		&& config_lookup_int(&cfg, "api_modules_number", &(c->api_modules_number))
	) {
		c->buffer = (char*) calloc((size_t) c->buffer_size, sizeof(char));

		/* Output a list of all movies in the inventory. */
		modules_setting = config_lookup(&cfg, "api_modules");
		if (modules_setting != NULL) {
			count = (unsigned int) config_setting_length(modules_setting);
			if (count != (unsigned int) c->api_modules_number)
				return CONFIG_INCONSISTENT_DATA;
			else if (count == 0)
				return CONFIG_FILE_READ_OK;

			c->api_modules_names = malloc(count * sizeof(char*));
			for (i = 0; i < count; ++i) {
				module_setting = config_setting_get_elem(modules_setting, i);
				module_name = config_setting_get_string(module_setting);

				c->api_modules_names[i] = module_name;
			}

			map_init(&c->api_modules, c->api_modules_number);
		}

		return CONFIG_FILE_READ_OK;
	}
	else
		return CONFIG_MISSING_KEY;
}
