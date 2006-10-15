
header=header.html_frag
footer=footer.html_frag 

README.html: README.txt $(header) $(footer)
	cat $(header) > $@
	Markdown.pl $< >> $@
	cat $(footer) >> $@


