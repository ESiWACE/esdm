SOURCES = $(wildcard *.dot)
OBJECTS = $(SOURCES:.dot=.png)

all: $(OBJECTS) Makefile

#dot -Tpdf $< -o $@.tmp
	#pdfcrop $@.tmp $@
	#rm $@.tmp
$(OBJECTS) : %.png: %.dot
	dot -Tpng $< -o $@

clean:
	rm $(OBJECTS)
