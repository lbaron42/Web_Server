FROM alpine:3.19 as builder

RUN apk update \
	&& apk upgrade \
	&& apk add coreutils gcc g++ make
WORKDIR /build
COPY . .
RUN make static

FROM scratch
LABEL authors="lbaron, mcutura, plandolf"
COPY --from=builder /build/webserv /usr/bin/webserv
COPY config/default.conf /etc/webserv/default.conf
ENTRYPOINT [ "/usr/bin/webserv" ]
CMD [ "/etc/webserv/default.conf" ]
EXPOSE 8080
