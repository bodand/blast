PICS := smart.pikchr gc.pikchr threads.pikchr
SVGS :=
PNGS :=
PDFS := $(PICS:.pikchr=.pdf) $(SVGS:.svg=.pdf) ${PNGS:.png=.pdf}
TEXS := time.tex mem.tex

INKSCAPE := inkscape
PIKCHR   := pikchr
GENPLOT  := genplot.py

.SUFFIXES:
.SUFFIXES: .pikchr .svg .pdf .png .rlg .tex

all: $(PDFS) ${TEXS}

clean:
	rm -f ${PDFS} ${TEXS}

.pikchr.svg:
	$(PIKCHR) --svg-only $< > $@

.svg.pdf:
	$(INKSCAPE) -D $< -o $@

.png.pdf:
	$(INKSCAPE) -D $< -o $@

time.tex mem.tex: ${GENPLOT} measure.rlg
	python3 ${GENPLOT} measure.rlg time.tex mem.tex

