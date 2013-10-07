#include "config.h"

s_config c;
char generated = 0;

s_config *get_config()
{
	if (generated == 0) {
		generated = 1;
		c.root = "/path/to/the/root/folder";
		c.port = 9999;
		c.host = "127.0.0.1";
	}
	return &c;
}
