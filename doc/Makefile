.PHONY: all clean assets latex

TEXDIR = ./latex
MDDIR = ./markdown

FILES = \
	$(filter-out main.tex, $(notdir $(wildcard $(TEXDIR)/*.tex))) \
	$(addprefix design/, $(notdir $(wildcard $(TEXDIR)/design/*.tex))) \
	$(addprefix user-guides/, $(notdir $(wildcard $(TEXDIR)/user-guides/*.tex)))

MDFILES = $(addprefix $(MDDIR)/, $(addsuffix .md, $(basename $(FILES))))

all: latex assets $(MDFILES) README.md 

assets:
	$(MAKE) -C assets/esdm-backends/mero
	$(MAKE) -C assets/esdm-backends/POSIX
	$(MAKE) -C assets/esdm-backends/WOS

EXTENSIONS=pipe_tables+simple_tables+fenced_code_blocks+fenced_code_attributes
PANDOC = pandoc --filter pandoc-crossref --citeproc -f latex

$(MDFILES) : $(MDDIR)/%.md : $(TEXDIR)/%.tex
	@mkdir -p $(@D)
	$(PANDOC) -t markdown+$(EXTENSIONS) $^ -o $@

README.md: latex/main.tex
	cd latex; $(PANDOC) -t gfm --lua-filter=image-path-filter.lua --table-of-contents --toc-depth=3 -f latex -s $(notdir $^) -o ../../README.md

latex:
	$(MAKE) -C latex

clean:
	$(MAKE) -C assets/esdm-backends/mero clean
	$(MAKE) -C assets/esdm-backends/POSIX clean
	$(MAKE) -C assets/esdm-backends/WOS clean
	rm ../README.md
	rm markdown -rf
	$(MAKE) -C latex clean
