CFLAGS=-Wall -Wextra -Werror -pedantic -pedantic-errors

ASSETS = umspec.txt codex.umz sandmark.umz um.um sandmark-output.txt

.PHONY: default
default: solve

$(ASSETS):
	curl -s http://www.boundvariable.org/$@ -o $@

.PHONY: clean
clean:
	rm -f vm vmd *.out *.um solution.pp *~

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

codex.out: vm codex.umz key
	./vm codex.umz < key > $@

codex.um: codex.out
	@dd skip=195 iflag=skip_bytes 2>/dev/null < $^ > $@

%.pp: %.txt
        # remove comments
	@grep -v "^#.*" < $^ > $@

.PHONY: solve
solve: vm codex.um solution.pp sandmark
	./vm codex.um < solution.pp

.PHONY: adventure
adventure: vm codex.um adventure.pp
	@./vm codex.um < adventure.pp

.PHONY: 2d
2d: vm codex.um 2d.pp
	@./vm codex.um < 2d.pp

.PHONY: run
run:
	./vm codex.um
