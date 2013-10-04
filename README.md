# C web server

A web server coded in C.

Provides an API and a web access.

## Requirements

libevent-dev

## Compilation

```
make
```

## Usage

```
./server
```


### Web

To use the web part, you'll need a web folder at the root of the project, then,
call http://localhost:9999/web/your/file

For example if you have in your web folder a test.html file, you can serve it:
http://localhost:9999/web/test.html

### API

To use the API part, you'll need to call http://localhost:9999/api

This section is not implemented yet.


