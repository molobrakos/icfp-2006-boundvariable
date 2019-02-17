CFLAGS=-Wall -Wextra -Werror -pedantic -pedantic-errors

ASSETS = umspec.txt codex.umz sandmark.umz um.um sandmark-output.txt

.PHONY: default clean tidy solve

default: solve

$(ASSETS):
	curl http://www.boundvariable.org/$@ -o $@

clean:
	rm -f vm vmd *.out *.um solution.pp *~

tidy: clean
	rm -f $(ASSETS)

sandmark.out: sandmark.umz vm
	@echo "running sandmark ..."
	time -v ./vm $< > $@

sandmark: sandmark.out sandmark-output.txt
	@diff $^

vm: vm.c
	gcc -DNDEBUG -Ofast $(CFLAGS) -o $@ $^

vmd: vm.c
	gcc -DDEBUG -Og -g $(CFLAGS) -o $@ $^

codex.out: vm codex.umz key
	./vm codex.umz < key > $@

codex.um: codex.out
	@dd skip=195 iflag=skip_bytes 2>/dev/null < $^ > $@

%.pp: %.txt
        # remove comments
	@grep -v "^#.*" < $^ > $@

solve: vm codex.um solution.pp
	./vm codex.um < solution.pp

adventure: vm codex.um adventure.pp
	@./vm codex.um < adventure.pp

2d: vm codex.um 2d.pp
	@./vm codex.um < 2d.pp

run:
	./vm codex.um
