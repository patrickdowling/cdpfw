TAG ?= patrickdowling/cdpfw-avr

.PHONY: build
build:
	docker build --tag $(TAG) .

.PHONY: clean
clean:
	docker image rm $(TAG) --force
	docker system prune --force
