.PHONY: all assets gfm_readme latex clean 

TEXDIR = ./latex
MDDIR = ./markdown

TEXFILES = \
	$(filter-out $(wildcard $(TEXDIR)/*main*.tex), $(wildcard $(TEXDIR)/*.tex)) \
	$(foreach directory, design user-guides, $(wildcard $(TEXDIR)/$(directory)/*.tex))

MDFILES = $(patsubst $(TEXDIR)/%, $(MDDIR)/%, $(addsuffix .md, $(basename $(TEXFILES))))

all: assets $(MDFILES) latex ../README.md 

assets:
	$(MAKE) -C assets/esdm-backends/mero
	$(MAKE) -C assets/esdm-backends/POSIX
	$(MAKE) -C assets/esdm-backends/WOS
	$(MAKE) -C assets/architecture
	$(MAKE) -C assets/deps
	$(MAKE) -C assets/build-docs-overview

EXTENSIONS=+pipe_tables-simple_tables-fenced_code_attributes
PANDOC = pandoc --filter pandoc-crossref --citeproc -M cref=true -f latex

$(MDFILES) : $(MDDIR)/%.md : $(TEXDIR)/%.tex
	@mkdir -p $(@D)
	cat ./md_header.tex $^ ./md_footer.tex | $(PANDOC) --lua-filter=markdown-image-path-filter.lua --bibliography=./latex/lib.bib --biblatex -t markdown$(EXTENSIONS) -o $@
	#$(PANDOC) --lua-filter=markdown-image-path-filter.lua --bibliography=latex/lib.bib --biblatex -t markdown$(EXTENSIONS) $^ -o $@

../README.md: latex/main.tex $(TEXFILES) ./figures/*
	cd latex; $(PANDOC) --lua-filter=image-path-filter.lua --table-of-contents --toc-depth=2 -s -t gfm main.tex -o ../$@

latex:
	$(MAKE) -C ./latex

clean:
	$(MAKE) -C assets/esdm-backends/mero clean
	$(MAKE) -C assets/esdm-backends/POSIX clean
	$(MAKE) -C assets/esdm-backends/WOS clean
	$(MAKE) -C assets/architecture clean
	$(MAKE) -C assets/deps clean
	$(MAKE) -C assets/build-docs-overview clean
	rm ../README.md
	rm markdown -rf
	$(MAKE) -C ./latex clean
