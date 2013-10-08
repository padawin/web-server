#include "config.h"

int get_config(s_config *c)
{
	c->root = "/path/to/the/root/folder";
	c->port = 9999;
	c->host = "127.0.0.1";

	return 0;
}
