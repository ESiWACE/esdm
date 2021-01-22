.PHONY: all

PUFILES = $(wildcard *.pu)
EPSFILES = $(addsuffix .eps, $(basename $(PUFILES)))

all: $(EPSFILES)

$(EPSFILES) : %.eps : %.pu
	plantuml -teps $^

clean:
	rm -rf *.eps
	rm -rf *-eps-converted-to.pdf
