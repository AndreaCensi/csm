
maruku=ruby -I~/maruku/lib ~/maruku/bin/maruku

all: csm_manual.pdf csm_manual.html

src=\
  preamble.txt \
  readme.txt \
  install.txt \
  laserdata.txt \
  formats.txt \
  other.txt 

csm_manual.txt: $(src)
	cat $^ > $@

%.pdf: %.txt
	$(maruku) --pdf -o $@ $<

%.html: %.txt
	$(maruku) --html $<


