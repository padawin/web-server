#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "evhttp.h"

short http_port = 9999;
char *http_addr = "127.0.0.1";
char* rootFolder;
short rootFolderSize;
struct evhttp *http_server = NULL;

char isWebCall(char **uri);
char isAPICall(char **uri);
short web_render_file(char* uri, struct evbuffer *evb);

void request_handler(struct evhttp_request *req, void *arg)
{
	int responseStatus;
	char* responseStatusText;
	struct evbuffer *evb;

	evb = evbuffer_new();
	fprintf(stdout, "Request for %s from %s\n", req->uri, req->remote_host);

	if (isWebCall(&req->uri)) {
		short rendered = web_render_file(req->uri, evb);

		if (rendered > 0) {
			responseStatus = HTTP_NOTFOUND;
			responseStatusText = "Not found";
		}
		else {
			responseStatus = HTTP_OK;
			responseStatusText = "OK";
		}
	}
	else if (isAPICall(&req->uri)) {
		evbuffer_add_printf(evb, "api");
		responseStatus = HTTP_OK;
		responseStatusText = "OK";
	}
	else {
		responseStatus = HTTP_NOTFOUND;
		responseStatusText = "Not found";
	}

	evhttp_send_reply(req, responseStatus, responseStatusText, evb);
	fprintf(stdout, "Response: %d %s\n", responseStatus, responseStatusText);
	evbuffer_free(evb);
	return;
}

short web_render_file(char* uri, struct evbuffer *evb)
{
	FILE* fp;
	char* buffer = NULL;
	size_t len = 0;

	int reqSize;

	reqSize = strlen(uri) + rootFolderSize + 1;
	char filepath[reqSize];
	snprintf(filepath, sizeof filepath, "%s%s", rootFolder, uri);

	//~char* filepath = (char*) malloc((sizeof(req->uri) + rootFolderSize) * sizeof(char));
	//~strcat(filepath, rootFolder);
	//~strcat(filepath, req->uri);

	fp = fopen(filepath, "r");
	if (!fp) {
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = malloc(len);
	if (buffer) {
		fread(buffer, 1, len, fp);
		evbuffer_add(evb, buffer, len);
	}
	fclose(fp);
	free(buffer);

	return 0;
}

char _is(char **uri, char* what)
{
	return strstr(*uri, what) - *uri == 0 && ((*uri)[4] == '\0' || (*uri)[4] == '/');
}

char isWebCall(char **uri)
{
	return _is(uri, "/web");
}

char isAPICall(char **uri)
{
	return _is(uri, "/api");
}

int main (int argc, const char * argv[])
{
	rootFolder = "/home/ghislain/dev-perso/server";
	rootFolderSize = (int) strlen("/home/ghislain/dev-perso/server");

	event_init();
	http_server = evhttp_start(http_addr, http_port);
	if (http_server == NULL) {
		fprintf(stderr, "Error starting http server on port %d\n", http_port);
		exit(1);
	}

	evhttp_set_gencb(http_server, request_handler, NULL);

	fprintf(stderr, "Server started on port %d\n", http_port);

	event_dispatch();

	return 0;
}
