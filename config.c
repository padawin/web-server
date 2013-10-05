#include "config.h"

s_config c;
char generated = 0;

s_config *get_config()
{
	if (generated == 0) {
		generated = 1;
		c.root = "/home/ghislain/projets/workspace/c/server";
		c.port = 9999;
		c.host = "127.0.0.1";
	}
	return &c;
}
