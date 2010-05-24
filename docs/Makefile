
#maruku=ruby -I~/maruku/lib ~/maruku/bin/maruku
maruku=maruku

out=csm_manual.pdf csm_manual.html

all: $(out)
	cp $(out) ..

src=\
  preamble.txt \
  inline_style.css \
  readme.txt \
  install.txt \
  install-ruby.txt \
  laserdata.txt \
  formats.txt \
  examples.txt \
  embedding.txt


%: tmp_%
	cp $< $@

csm_manual.txt: $(src)
	cat $^ > $@

%.pdf: %.txt
	$(maruku) --pdf -o $@ $<

%.html: %.txt
	$(maruku) --html $<

clean: 
	rm $(out)
	
copy: out/csm_manual.html
	cp $< ../CSM_MANUAL.html

