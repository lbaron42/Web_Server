function FindProxyForURL(url, host) {
    if (url.startsWith("http:")) {
        if (host=='marvinx.42.fr'){
            return 'HTTP 127.0.0.1:8080';
        }
        if (host=='putchar.mc'){
            return 'HTTP 127.0.0.1:8080';
        }
        if (host=='ping.js'){
            return 'HTTP 127.0.0.1:8080';
        }
        if (host=='pong.js'){
            return 'HTTP 127.0.0.1:8080';
        }
    }
    // All other domains should connect directly without a proxy
    return "DIRECT";
}
