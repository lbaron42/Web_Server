FROM alpine:3.19 as builder
LABEL authors="lbaron, mcutura, plandolf"

RUN apk update \
	&& apk upgrade \
	&& apk add coreutils gcc g++ make
WORKDIR /build
COPY . .
RUN make deploy

FROM scratch
COPY --from=builder /build/webserv /bin/webserv
COPY config/default.conf /etc/webserv/default.conf
ENTRYPOINT [ "/bin/webserv" ]
CMD [ "/etc/webserv/default.conf" ]
EXPOSE 8080
