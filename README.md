# Web_Server

WIP

![build](https://github.com/lbaron42/Web_Server/actions/workflows/build.yml/badge.svg)

## Deploy

```
docker build . -t webserv \
&& docker run --name c-webserv -p 8080:8080 -it webserv
```
Server now available at 127.0.0.1:8080
