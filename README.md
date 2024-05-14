# Web_Server

WIP

## Deploy

```
docker build . -t webserv \
&& docker run --name c-webserv -p 8080:8080 -it webserv
```
Server now available at 127.0.0.1:8080
