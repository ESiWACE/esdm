.PHONY: all assets gfm_readme latex clean 

TEXDIR = ./latex
MDDIR = ./markdown

TEXFILES = \
	$(filter-out latex/main.tex, $(wildcard $(TEXDIR)/*.tex)) \
	$(foreach directory, design user-guides, $(wildcard $(TEXDIR)/$(directory)/*.tex))

MDFILES = $(patsubst $(TEXDIR)/%, $(MDDIR)/%, $(addsuffix .md, $(basename $(TEXFILES))))

all: latex assets $(MDFILES) ../README.md 

assets:
	$(MAKE) -C assets/esdm-backends/mero
	$(MAKE) -C assets/esdm-backends/POSIX
	$(MAKE) -C assets/esdm-backends/WOS
	$(MAKE) -C assets/architecture

EXTENSIONS=pipe_tables+simple_tables+fenced_code_blocks+fenced_code_attributes
PANDOC = pandoc --filter pandoc-crossref --citeproc -f latex

$(MDFILES) : $(MDDIR)/%.md : $(TEXDIR)/%.tex
	@mkdir -p $(@D)
	$(PANDOC) -t markdown+$(EXTENSIONS) $^ -o $@

../README.md: latex/main.tex $(TEXFILES) ./figures/*
	cd latex; $(PANDOC) -t gfm --lua-filter=image-path-filter.lua --table-of-contents --toc-depth=3 -s main.tex -o ../$@

latex:
	$(MAKE) -C latex

clean:
	$(MAKE) -C assets/esdm-backends/mero clean
	$(MAKE) -C assets/esdm-backends/POSIX clean
	$(MAKE) -C assets/esdm-backends/WOS clean
	$(MAKE) -C assets/architecture clean
	rm ../README.md
	rm markdown -rf
	$(MAKE) -C latex clean
