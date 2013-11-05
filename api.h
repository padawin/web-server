#include "config.h"

void *api_open_module(const char *module_name, s_config *conf);
char *api_run_module(void *module, const char *module_name, const char *callback);
const char *api_get_method(struct evhttp_request *req);
short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf);
