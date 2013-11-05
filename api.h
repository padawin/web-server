#include "config.h"

void *open_api_module(const char *module_name, s_config *conf);
char *run_api_module(void *module, const char *module_name, const char *callback);
const char *get_method(struct evhttp_request *req);
short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf);
