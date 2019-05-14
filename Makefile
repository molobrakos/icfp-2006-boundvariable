CFLAGS=-Wall -Wextra -Werror -pedantic -pedantic-errors -std=c99

ASSETS = umspec.txt codex.umz sandmark.umz um.um sandmark-output.txt

VM = ./vm -e codex.um
PREPROCESS = ./preprocess


.PHONY: default
default: solve

$(ASSETS):
	curl -s http://www.boundvariable.org/$@ -o $@

.PHONY: clean
clean:
	rm -f vm vmd *.out *.um *~

.PHONY: realclean
ealclean: clean
	rm -f $(ASSETS)

sandmark.out: sandmark.umz vm
	@echo "running sandmark ..."
	time -v ./vm $< > $@

.PHONY: sandmark
sandmark: sandmark.out sandmark-output.txt
	@diff $^

vm: vm.c
	gcc -DNDEBUG -Ofast $(CFLAGS) -o $@ $^

vmd: vm.c
	gcc -DDEBUG -Og -g $(CFLAGS) -o $@ $^

codex.um: vm codex.umz key
	./vm codex.umz < key | dd skip=195 iflag=skip_bytes > $@

.PHONT: basic
basic: script/basic.script hack.bas vm codex.um
	cat $< | $(PREPROCESS) | $(VM)

.PHONY: crack
crack:
	make basic | grep "^!!! cracked"

.PHONY: adventure
adventure: script/adventure.script vm codex.um
	cat $< | $(PREPROCESS) | $(VM)

.PHONY: adventure
2d: script/2d.script vm codex.um
	cat $< | $(PREPROCESS) | $(VM)

.PHONY: certify
certify: script/certify.script vm codex.um
	cat $< | $(PREPROCESS) | $(VM)

.PHONY: advice
advise: script/advise.script vm codex.um
	cat $< | $(PREPROCESS) | $(VM)

.PHONY: solve
solve: basic adventure 2d certify advise

.PHONY: cookies
cookies:
	make | grep '\w\{5\}\.\w\{3\}='

.PHONY: run
run:
	./vm codex.um
