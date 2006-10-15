
output=egsl.html

htmlsection=./htmlsection -no-spacer
markdown=Markdown.pl

header=header.html_frag
footer=footer.html_frag 
tmp=tmp


all: $(output)

$(output): README.txt $(header) $(footer)
	$(markdown) $< | $(htmlsection) > $(tmp)
	cat $(header) $(tmp) $(footer) > $@
	-rm $(tmp)

clean:
	-rm $(output)