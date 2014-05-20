# C web server

A web server coded in C.

Provides an API and a web access.

## Requirements

* libevent-dev
* libconfig-dev
* libmap (https://github.com/padawin/map-structure-implementation)

## Usage

In config.cfg, change the values to match your environment.

Compile it:
```
make
make install
```

## Usage

```
./server
```


### Web

To use the web part, you'll need a web folder and its path must be in the
web_root variable in the config.cfg.

For example if you have in your web folder a test.html file, you can serve it:
http://localhost:port/web/test.html

### API

## Description

To use the API part, you'll need to call http://localhost:port/api

The api uses .so modules. The modules must be stored in the directory defined in
configuration key 'api_modules_path'.

A module must have this include:
```
#include "webserver/module.h"
```

This provides the signatures for the functions:
```
* char *get_call(void);
* char *post_call(void);
* char *get_call(void);
* char *post_call(void);
* char *head_call(void);
* char *put_call(void);
* char *delete_call(void);
* char *options_call(void);
* char *trace_call(void);
* char *connect_call(void);
* char *patch_call(void);
* char *get_call(void);
* char *post_call(void);
* char *get_call(void);
* char *post_call(void);
* char *head_call(void);
* char *put_call(void);
* char *delete_call(void);
* char *options_call(void);
* char *trace_call(void);
* char *connect_call(void);
* char *patch_call(void);
```

Which are called depending on the HTTP request method.
The functions' return values are the response body.

## Compilation

To compile a module, use the server's Makefile:
```
make /path/to/your/module.so
```

The module's source must be in /path/to/your/module.c.

## Usage

To use a module, it must be registered in the configuration file in the key
'api_modules' and 'api_modules_number'. For example, if you have two modules
"foo.so" and "bar.so", your configuration file must look like:
```
api_modules_number = 2;
api_modules: ("foo", "bar");
```
And then restart the server and call http://localhost:port/api/%modulename%

## Example of module

```
#include <webserver/module.h>
#include <map.h>
#include <stdlib.h>

char *get_call(map *params)
{
	char *foo;

	if ((foo = map_get_entry("foo", params)) != NULL) {
		return foo;
	}

	return (char *) "foo";
}
```
