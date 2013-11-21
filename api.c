// For dlopen
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <evhttp.h>
#include "api.h"

// Limit to 255 the total number of parameters, which is already huge...
#define TOTAL_PARAMS_NUMBER 255

short decompose_uri(const char *uri, s_config *conf, char *module, char *params);

/**
 * Function used as a callback to be called when a request to the API is received
 *
 * @param struct evhttp_request *req The request
 * @param struct evbuffer *evb The response buffer
 * @param s_config *conf The server configuration
 * @return short 0 if everything went fine, -1 else
 */
short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf)
{
	const char *cb;

	char *params, *module, *response;
	short detect_module_result;
	struct evkeyvalq *headers;
	struct evkeyval *header;

	module = calloc(1, sizeof(char *));
	params = calloc(1, sizeof(char *));
	if ((detect_module_result = decompose_uri(req->uri, conf, module, params)) != 0) {
		return detect_module_result;
	}

	// Get .so to execute
	void *loaded_module = map_get_entry(module, &conf->api_modules);
	if (loaded_module == NULL) {
		free(module);
		free(params);
		return -1;
	}

	// get arguments
	if (params != NULL) {
		headers = malloc(sizeof(struct evkeyvalq*));
		evhttp_parse_query_str(&params[1], headers);
		for (header = headers->tqh_first; header; header = header->next.tqe_next) {
			printf("--%s \"%s\"\n", header->key, header->value);
		}
		free(headers);
	}

	// Get method
	cb = api_get_method(req);

	response = api_run_module(loaded_module, module, cb);
	free(module);
	free(params);
	if (response == NULL) {
		return -1;
	}

	// Print the result
	evbuffer_add_printf(evb, "%s", response);
	return 0;
}

/**
 * Function to extract the module name and the parameters from the uri.
 *
 * @param const char *uri The uri to analyse
 * @param s_config *conf The server configuration
 * @param char *module Variable where the module name will be stored
 * @param char *params Variable where the params string will be stored
 * @return short 0 if everything went fine, -1 else
 */
short decompose_uri(const char *uri, s_config *conf, char *module, char *params)
{
	int uriStartChar;
	char *reduced_uri, *p;
	long unsigned int moduleLen;

	// Remove the "/[conf->api_prefix]/" of the uri
	uriStartChar = (int) strlen(conf->api_prefix);
	reduced_uri = (char *) &uri[uriStartChar];

	// no module provided, full uri like /api or /api/
	if (
		strlen(reduced_uri) <= 1 ||
		// uri like ?foo
		reduced_uri[0] == '?' ||
		// uri like /?foo
		reduced_uri[1] == '?' ||
		//uri like //foo
		reduced_uri[1] == '/'
	) {
		return -1;
	}

	// remove left training / in uri
	reduced_uri = &reduced_uri[1];
	// The module is the substring before the next /
	p = strchr(reduced_uri, '/');
	// or the substring before the next question mark
	if (p == NULL) {
		p = strchr(reduced_uri, '?');
	}

	if (p == NULL)
		moduleLen = strlen(reduced_uri);
	else {
		moduleLen = (long unsigned int) (p - reduced_uri);
		strncpy(params, p, strlen(p));
	}

	strncpy(module, reduced_uri, moduleLen);

	return 0;
}

/**
 * Function to get the request HTTP method
 *
 * @param struct evhttp_request *req The request
 * @return const char* The method name
 */
const char *api_get_method(struct evhttp_request *req)
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

/**
 * Function to run the desired API module.
 * The function is defined from the request method.
 * For example a get method will try to call get_call()
 *
 * @param void *module The requested module
 * @param const char *module_name The requested module name, used only when an
 * 		error occurs
 * @param const char *callback The method name, from that, the function name will
 * 		be defined.
 * @return char* The return value of the called function. Or null if the function
 * 		does not exist.
 */
char *api_run_module(void *module, const char *module_name, const char *callback)
{
	const unsigned short int cb_size = 13;
	char *result;
	char module_cb[cb_size];

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

/**
 * Function to open a .so module from its name.
 *
 * @param const char *module_name The module name
 * @param s_config *conf The server configuration
 * @return void* The loaded module
 */
void *api_open_module(const char *module_name, s_config *conf)
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
