CFLAGS=-Wall -Wextra -Werror -pedantic -pedantic-errors

ASSETS = umspec.txt codex.umz sandmark.umz um.um sandmark-output.txt

VM = ./vm codex.um

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
basic: basic.script hack.bas $(VM)
	cat $< | ./pp | $(VM)

.PHONY: crack
crack:
	make basic | grep "^!!! cracked"

.PHONY: adventure
adventure: adventure.script $(VM)
	cat $< | ./pp | $(VM)

.PHONY: adventure
2d: 2d.script $(VM)
	cat $< | ./pp | $(VM)

.PHONY: solve
solve: basic adventure 2d

.PHONY: run
run:
	./vm codex.um
