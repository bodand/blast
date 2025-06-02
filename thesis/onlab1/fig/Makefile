PICS := uneval-param.pikchr eval-param.pikchr
SVGS := 
PDFS := $(PICS:.pikchr=.pdf) $(SVGS:.svg=.pdf)

INKSCAPE := "C:\Program Files\Inkscape\bin\inkscape.exe" 
PIKCHR   := pikchr

.SUFFIXES:
.SUFFIXES: .pikchr .svg .pdf

all: $(PDFS)

svg: $(PICS:.pikchr=.svg)

.pikchr.svg: 
	$(PIKCHR) --svg-only $< > $@

.svg.pdf:
	$(INKSCAPE) -D $< -o $@

