#!/usr/bin/bash

xargs -I % -P 5 curl -H "Host: putchar.mc" -H "Connection: keep-alive" \
--resolve putchar.mc:8080:127.0.0.1 \
putchar.mc:8080/x/parallel.cgi \
< <(printf '%s\n' {1..5})
