PICS := ast.pikchr ffm.pikchr ssa.pikchr bad-memory.pikchr bits.pikchr \
				fn-mem.pikchr package-mem.pikchr
SVGS := 
PDFS := $(PICS:.pikchr=.pdf) $(SVGS:.svg=.pdf)

INKSCAPE := "C:\Program Files\Inkscape\bin\inkscape.exe" 
PIKCHR   := pikchr

.SUFFIXES:
.SUFFIXES: .pikchr .svg .pdf

all: $(PDFS)

.pikchr.svg: 
	$(PIKCHR) --svg-only $< > $@

.svg.pdf:
	$(INKSCAPE) -D $< -o $@

