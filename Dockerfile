FROM alpine:3.19 as builder

RUN apk update \
	&& apk upgrade \
	&& apk add coreutils gcc g++ make
WORKDIR /build
COPY src src
COPY include include
COPY Makefile Makefile
RUN make static

FROM scratch
LABEL authors="lbaron, mcutura, plandolf"
COPY --from=builder /build/webserv /usr/bin/webserv
COPY config/container.conf /etc/webserv/container.conf
ENTRYPOINT [ "/usr/bin/webserv" ]
CMD [ "/etc/webserv/container.conf" ]
EXPOSE 8080
