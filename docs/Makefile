
maruku=ruby -I~/maruku/lib ~/maruku/bin/maruku

all: csm_manual.pdf csm_manual.html

src=\
  preamble.txt \
  readme.txt \
  install.txt \
  install-ruby.txt \
  laserdata.txt \
  formats.txt \
  examples.txt 

csm_manual.txt: $(src)
	cat $^ > $@
	chmod -w $@

%.pdf: %.txt
	$(maruku) --pdf -o $@ $<

%.html: %.txt
	$(maruku) --html $<


