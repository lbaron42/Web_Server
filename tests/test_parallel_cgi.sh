#!/usr/bin/bash

xargs -I % -P 5 curl -H "Host: putchar.mc" -H "Connection: keep-alive" \
localhost:8080/test-cgi/parallel.cgi \
< <(printf '%s\n' {1..5})
