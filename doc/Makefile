.PHONY: all clean

INDIR = ./latex
MDDIR = ./doxygen

FILES = \
	$(notdir $(wildcard $(INDIR)/*.tex)) \
	$(addprefix design/, $(notdir $(wildcard $(INDIR)/design/*.tex))) \
	$(addprefix user-guides/, $(notdir $(wildcard $(INDIR)/user-guides/*.tex)))

MDFILES = $(addprefix $(MDDIR)/, $(addsuffix .md, $(basename $(FILES))))

all: README.md $(MDFILES) 

#PLANTUML = plantuml -teps
#$(OUTDIR)/%.eps: $(INDIR)/%.pu
#  @mkdir -p $(@D)
#  $(PLANTUML) -o$(shell pwd)/$(dir $@) $^

EXTENSIONS=pipe_tables+simple_tables+fenced_code_blocks+fenced_code_attributes
PANDOC = pandoc --filter pandoc-crossref --citeproc -f latex

$(MDDIR)/main.md: $(INDIR)/main.tex
	@echo Skip main.tex. Not required for doxygen documentation.	

$(MDDIR)/%.md: $(INDIR)/%.tex
	@mkdir -p $(@D)
	$(PANDOC) -t markdown+$(EXTENSIONS) $^ -o $@

README.md: latex/main.tex
	cd latex; $(PANDOC) -t gfm --table-of-contents --toc-depth=3 -f latex -s $(notdir $^) -o ../../README.md
