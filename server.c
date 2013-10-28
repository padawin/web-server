#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// stat call
#include <sys/stat.h>
#include <event.h>
#include <evhttp.h>
#include "config.h"

/**
 * Signatures
 */
char _is(char **uri, const char* what, int whatLength);
char isWebCall(char **uri, s_config *conf);
char isAPICall(char **uri, s_config *conf);
short web_render_file(char* uri, struct evbuffer *evb, s_config *conf);
short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf);
void request_handler(struct evhttp_request *req, void *conf);
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
 * Function to render a static file in a web call
 */
short web_render_file(char* uri, struct evbuffer *evb, s_config *conf)
{
	FILE* fp;
	char *filepath, *cFilePath;
	struct stat fs;
	int nbChars, fInfo;
	short rootFolderSize;

	filepath = NULL;
	cFilePath = NULL;
	rootFolderSize = (short) strlen(conf->web_root);

	nbChars = rootFolderSize + (int) strlen(uri) + 1;
	filepath = (char*) calloc((size_t) nbChars, sizeof(char));

	strcat(filepath, conf->web_root);
	strcat(filepath, &uri[strlen(conf->web_prefix)]);
	cFilePath = realpath(filepath, cFilePath);
	free(filepath);

	if (cFilePath == NULL || strstr(cFilePath, conf->web_root) == NULL) {
		return -1;
	}

	// get some infos on the file
	fInfo = stat(cFilePath, &fs);

	if (fInfo == -1) {
		// @TODO check if the file does not exist
		// errno == ENOENT => 404
		return -1;
	}

	if ((fs.st_mode & S_IFDIR) == S_IFDIR) {
		const char* dFile = conf->index_file;
		strcat(cFilePath, "/");
		strcat(cFilePath, dFile);
		// the new file is the index.html in the directory, let's stat again
		fInfo = stat(cFilePath, &fs);
	}

	fp = fopen(cFilePath, "r");

	if (!fp) {
		return -1;
	}

	size_t sBuff;
	while (!feof(fp)) {
		sBuff = fread(conf->buffer, 1, (size_t) conf->buffer_size, fp);
		evbuffer_add(evb, conf->buffer, sBuff);
	}

	fclose(fp);

	return 0;
}

short api_cb(struct evhttp_request *req, struct evbuffer *evb, s_config *conf)
{
	char *uri, *module;
	int uriStartChar, moduleLen;

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

	return 0;
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

int main()
{
	int http_port;
	const char *http_addr;
	struct evhttp *http_server;
	s_config *c;

	c = (s_config*) malloc(sizeof(s_config));

	int confRet;
	if ((confRet = get_server_config(c)) != CONFIG_FILE_READ_OK) {
		fprintf(stderr, "Error while reading the configuration file, error %d\n", confRet);
		exit(1);
	}

	c->buffer = (char*) calloc((size_t) c->buffer_size, sizeof(char));

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

	return 0;
}
