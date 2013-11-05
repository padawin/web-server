#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <evhttp.h>
#include "api.h"

short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf)
{
	const char *cb;

	char *uri, *module, *response;
	char found;
	int uriStartChar, moduleLen, moduleIndex;

	// Remove the "/[conf->api_prefix]/" of the uri
	uriStartChar = (int) strlen(conf->api_prefix);
	uri = &req->uri[uriStartChar];

	// no module provided, full uri like /api or /api/
	if (
		strlen(uri) <= 1 ||
		// uri like ?foo
		uri[0] == '?' ||
		// uri like /?foo
		uri[1] == '?' ||
		//uri like //foo
		uri[1] == '/'
	) {
		return -1;
	}

	// remove left training / in uri
	uri = &uri[1];
	// The module is the substring before the next /
	module = strchr(uri, '/');
	// or the substring before the next question mark
	if (module == NULL) {
		module = strchr(uri, '?');
	}

	if (module == NULL) {
		module = uri;
	}
	else {
		moduleLen = (int) (module - uri);
		module = uri;
		module[moduleLen] = '\0';
	}

	// Get .so to execute
	found = 0;
	for (moduleIndex = 0; moduleIndex < conf->api_modules_number && !found; ++moduleIndex) {
		if (strcmp(conf->api_modules[moduleIndex], module) == 0) {
			found = 1;
		}
	}

	if (!found) {
		return -1;
	}

	void *loaded_module = open_api_module(module, conf);
	if (loaded_module == NULL) {
		return -1;
	}


	// Get method
	cb = get_method(req);

	response = run_api_module(loaded_module, module, cb);
	if (response == NULL) {
		return -1;
	}

	// Print the result
	evbuffer_add_printf(evb, "%s", response);
	return 0;
}

const char *get_method(struct evhttp_request *req)
{
	const char *cb;

	cb = 0;
	switch (evhttp_request_get_command(req)) {
		case EVHTTP_REQ_GET:
			cb = "get";
			break;
		case EVHTTP_REQ_POST:
			cb = "post";
			break;
		case EVHTTP_REQ_HEAD:
			cb = "head";
			break;
		case EVHTTP_REQ_PUT:
			cb = "put";
			break;
		case EVHTTP_REQ_DELETE:
			cb = "delete";
			break;
		case EVHTTP_REQ_OPTIONS:
			cb = "options";
			break;
		case EVHTTP_REQ_TRACE:
			cb = "trace";
			break;
		case EVHTTP_REQ_CONNECT:
			cb = "connect";
			break;
		case EVHTTP_REQ_PATCH:
			cb = "patch";
			break;
		default:
			break;
	}

	return cb;
}

char *run_api_module(void *module, const char *module_name, const char *callback)
{
	const unsigned short int cb_size = 13;
	char *result;
	char module_cb[13];

	typedef char *(*query_f) ();
	query_f query;

	snprintf(module_cb, cb_size, "%s_call", callback);

	query = dlsym(module, module_cb);
	result = dlerror();
	if (result) {
		printf("Cannot find %s in %s: %s\n", module_cb, module_name, result);
		return NULL;
	}

	return query();
}

void *open_api_module(const char *module_name, s_config *conf)
{
	char module_file_name[80];
	void *plugin;

	sprintf(module_file_name, "%s/%s.so", conf->api_modules_path, module_name);
	plugin = dlopen(module_file_name, RTLD_NOW);
	if (!plugin) {
		printf("Cannot load %s: %s\n", module_name, dlerror());
		return NULL;
	}

	return plugin;
}
