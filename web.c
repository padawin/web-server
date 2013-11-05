#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <evhttp.h>
#include <stdlib.h>
// stat call
#include <sys/stat.h>
#include "web.h"

/**
 * Function to render a static file in a web call
 */
short web_render_file(const char* uri, struct evbuffer *evb, s_config *conf)
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
