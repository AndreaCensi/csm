
#maruku=ruby -I~/maruku/lib ~/maruku/bin/maruku
maruku=maruku

out=out/csm_manual.pdf out/csm_manual.html

all: $(out)


src=\
  preamble.txt \
  inline_style.css \
  readme.txt \
  install.txt \
  install-ruby.txt \
  laserdata.txt \
  formats.txt \
  examples.txt 


%: tmp_%
	cp $< $@

out/csm_manual.txt: $(src)
	mkdir -p out
	cat $^ > $@

%.pdf: %.txt
	$(maruku) --pdf -o $@ $<

%.html: %.txt
	$(maruku) --html $<

clean: 
	rm $(out)
	
copy: out/csm_manual.html
	cp $< ../CSM_MANUAL.html

