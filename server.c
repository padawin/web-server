#include <stdlib.h>
#include <string.h>
#include <evhttp.h>

// From https://github.com/padawin/map-structure-implementation
#include <map.h>

#include "api.h"
#include "web.h"
#include "config.h"

#define APP_NAME "zish"

/**
 * Signatures
 */
char _is(char **uri, const char* what, int whatLength);
char isWebCall(char **uri, s_config *conf);
char isAPICall(char **uri, s_config *conf);
void request_handler(struct evhttp_request *req, void *conf);
void load_api_modules(s_config *conf);
void get_configuration_filepath(char *path, const unsigned short int path_size);
void send_reply(
	struct evhttp_request *req,
	struct evbuffer *evb,
	short status,
	const char *status_text
);


/**
 * Callback to handle a request.
 * Will check if it is a web call, or an API call
 * Else a 404 error will be raised.
 *
 * A web call is a request starting with /web/
 * A API call is a request starting with /api/
 */
void request_handler(struct evhttp_request *req, void *conf)
{
	short responseStatus;
	const char* responseStatusText;
	struct evbuffer *evb;

	evb = evbuffer_new();
	fprintf(stdout, "Request for %s from %s\n", req->uri, req->remote_host);

	if (isWebCall(&req->uri, conf)) {
		short rendered = web_render_file(req->uri, evb, conf);

		if (rendered < 0) {
			responseStatus = HTTP_NOTFOUND;
			responseStatusText = "Not found";
		}
		else {
			responseStatus = HTTP_OK;
			responseStatusText = "OK";
		}
	}
	else if (isAPICall(&req->uri, conf)) {
		short result = api_cb(req, evb, conf);

		if (result < 0) {
			responseStatus = HTTP_NOTFOUND;
			responseStatusText = "Not found";
		}
		else {
			responseStatus = HTTP_OK;
			responseStatusText = "OK";
		}
	}
	else {
		responseStatus = HTTP_NOTFOUND;
		responseStatusText = "Not found";
	}

	send_reply(req, evb, responseStatus, responseStatusText);
}

void send_reply(
	struct evhttp_request *req,
	struct evbuffer *evb,
	short status,
	const char *status_text
)
{
	if (status != HTTP_OK) {
		evhttp_send_error(req, status, status_text);
	}
	else {
		evhttp_send_reply(req, status, status_text, evb);
	}

	fprintf(stdout, "Response: %d %s\n", status, status_text);
	evbuffer_free(evb);
}



/**
 * Function to know if a request starts with the what parameter.
 *
 * To know if the request is a web call, or an API call.
 */
char _is(char **uri, const char* what, int whatLength)
{
	return strstr(*uri, what) - *uri == 0 && ((*uri)[whatLength] == '\0' || (*uri)[whatLength] == '/' || (*uri)[whatLength] == '?');
}

/**
 * Function to know if the request is a web call
 */
char isWebCall(char **uri, s_config *conf)
{
	return _is(uri, conf->web_prefix, (int) strlen(conf->web_prefix));
}

/**
 * Function to know if the request is a API call
 */
char isAPICall(char **uri, s_config *conf)
{
	return _is(uri, conf->api_prefix, (int) strlen(conf->api_prefix));
}

/**
 * Function to load in memory all the declared api modules, from the
 * configuration.
 *
 * @param void *conf the configuration
 * @return void
 */
void load_api_modules(s_config *conf)
{
	int m;

	for (m = 0; m < conf->api_modules_number; ++m) {
		map_add_entry(
			conf->api_modules_names[m],
			api_open_module(conf->api_modules_names[m], conf),
			&conf->api_modules
		);
	}
}

/**
 * Function to get the path to the default configuration file to use
 *
 * @param char *path String where the config path will be stored
 * @param const unsigned short int path_size Total path size
 * @return void
 */
void get_configuration_filepath(char *path, const unsigned short int path_size)
{
	snprintf(path, path_size, "/etc/%s/%s.conf", APP_NAME, APP_NAME);
}

int main()
{
	int http_port;
	const char *http_addr;
	struct evhttp *http_server;
	s_config *c;

	c = (s_config*) malloc(sizeof(s_config));

	const unsigned short int path_size = 50;
	char conf_path[path_size];

	get_configuration_filepath(conf_path, path_size);

	int confRet;
	if ((confRet = get_server_config(c, conf_path)) != CONFIG_FILE_READ_OK) {
		fprintf(stderr, "Error while reading the configuration file, error %d\n", confRet);
		exit(1);
	}

	load_api_modules(c);

	http_server = NULL;
	http_addr = c->host;
	http_port = c->port;

	event_init();
	http_server = evhttp_start(http_addr, (short unsigned int) http_port);
	if (http_server == NULL) {
		fprintf(stderr, "Error starting http server on port %d\n", http_port);
		exit(1);
	}

	evhttp_set_gencb(http_server, request_handler, c);

	fprintf(stdout, "Server started on port %d\n", http_port);

	event_dispatch();

	free(c->buffer);
	free(c->api_modules_names);
	map_free(&c->api_modules);

	return 0;
}
