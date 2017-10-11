SOURCES = $(wildcard *.dot)
OBJECTS = $(SOURCES:.dot=.pdf)

all: $(OBJECTS) Makefile

%.pdf: %.dot
	dot -Tpdf $< -o $@.tmp
	pdfcrop $@.tmp $@
	rm $@.tmp

clean:
	rm $(OBJECTS)
