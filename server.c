#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// stat call
#include <sys/stat.h>
#include "event.h"
#include "evhttp.h"

char* rootFolder;
short rootFolderSize;

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

		if (rendered < 0) {
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
	char *buffer, *filepath, *cFilePath;
	size_t len;
	struct stat fs;
	int nbChars, fInfo;

	buffer = NULL;
	filepath = NULL;
	cFilePath = NULL;

	nbChars = rootFolderSize + (int) strlen(uri) + 1;
	filepath = (char*) calloc(nbChars, sizeof(char));

	strcat(filepath, rootFolder);
	strcat(filepath, uri);
	cFilePath = realpath(filepath, cFilePath);
	free(filepath);

	if (strstr(cFilePath, rootFolder) == NULL) {
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
		char* dFile = "/index.html";
		// 11 = strlen("/index.html")
		strcat(cFilePath, dFile);
		// the new file is the index.html in the directory, let's stat again
		fInfo = stat(cFilePath, &fs);
	}

	fp = fopen(cFilePath, "r");

	if (!fp) {
		return -1;
	}

	len = fs.st_size;
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
	// @XXX FIXME 4 must be dynamic
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

int main(int argc, const char * argv[])
{
	short http_port = 9999;
	char* http_addr = "127.0.0.1";
	struct evhttp *http_server = NULL;

	rootFolder = "/home/ghislain/dev-perso/server";
	rootFolderSize = (int) strlen(rootFolder);

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
