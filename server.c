#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// stat call
#include <sys/stat.h>
#include "event.h"
#include "evhttp.h"
#include "config.h"

/**
 * Signatures
 */
char _is(char **uri, const char* what, int whatLength);
char isWebCall(char **uri, s_config *conf);
char isAPICall(char **uri, s_config *conf);
short web_render_file(char* uri, struct evbuffer *evb, s_config *conf);
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
 *
 * A web call will serve static files stored in /web folder
 * @TODO This has to be moved in config
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
		evbuffer_add_printf(evb, "api");
		responseStatus = HTTP_OK;
		responseStatusText = "OK";
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
	char *buffer, *filepath, *cFilePath;
	size_t len;
	struct stat fs;
	int nbChars, fInfo;
	short rootFolderSize;

	buffer = NULL;
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

	len = (size_t) fs.st_size;
	buffer = malloc(len);
	if (buffer) {
		fread(buffer, 1, len, fp);
		evbuffer_add(evb, buffer, len);
	}

	fclose(fp);
	free(buffer);

	return 0;
}

/**
 * Function to know if a request starts with the what parameter.
 *
 * To know if the request is a web call, or an API call.
 */
char _is(char **uri, const char* what, int whatLength)
{
	return strstr(*uri, what) - *uri == 0 && ((*uri)[whatLength] == '\0' || (*uri)[whatLength] == '/');
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
	s_config c;

	if (get_server_config(&c) != CONFIG_FILE_READ_OK) {
		exit(1);
	}

	http_server = NULL;
	http_addr = c.host;
	http_port = c.port;

	event_init();
	http_server = evhttp_start(http_addr, (short unsigned int) http_port);
	if (http_server == NULL) {
		fprintf(stderr, "Error starting http server on port %d\n", http_port);
		exit(1);
	}

	evhttp_set_gencb(http_server, request_handler, &c);

	fprintf(stderr, "Server started on port %d\n", http_port);

	event_dispatch();

	return 0;
}
