LABEL org.opencontainers.image.source https://github.com/patrickdowling/cdp
FROM alpine:3.15

RUN apk --no-cache add \
	ca-certificates\
	make avr-libc gcc-avr cppcheck

WORKDIR /build

