function FindProxyForURL(url, host) {
    host = host.toLowerCase();
    if (dnsDomainIs(host, "marvinx.42.fr")
    || dnsDomainIs(host, "putchar.mc")
    || dnsDomainIs(host, "ping.js")
    || dnsDomainIs(host, "pong.js")) {
        return "HTTP 127.0.0.1:8080";
    } else
        return "DIRECT";
}
