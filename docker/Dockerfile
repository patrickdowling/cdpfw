FROM alpine:3.15
LABEL org.opencontainers.image.source https://github.com/patrickdowling/cdp

RUN apk --no-cache add \
	ca-certificates\
	make avr-libc gcc-avr cppcheck

WORKDIR /build

