
The Canonical Scan Matcher v.1.0.0
================================

* TOC
{:toc}

## Content of this package ##

This package contains a lot of software, but only some of that
is in a complete and usable state.


### Stable things: C scan matching library ###

The directory `sm/csm` contains a scan matcher written in C, plus 
associated tools and apps. This is stable and might be used.

There are many libraries in the `sm/lib` directory:

-	Directory `gpc`: a library for solving point-to-line correspondence
	problems. This is documented in another file.

-	Directory `egsl`: a light wrapper for GSL that makes manipulating
	matrices easy and efficient. This is documented in another file:
	see `sm/lib/egsl/docs`.

-	Directory `options`: for processing command-line arguments and 
	configuration files.

-	Directory `json-c`: a library for JSON input/output. This is a 
	slightly modified version of the original [`json-c`][json-c] 
	library released under the [MIT license].

### Stable things: applications ###

There are many applications in the `sm/apps` directory:

-	Application `carmen2pdf`: converts a laser log to a PDF map.
	To build this application, it is needed to install the [Cairo] graphics
	library.

-	Application `carmen2json`: 

-	Application `json_extract`:

-	Application `json_pipe`:

-	Application `ld_fisher`:

-	Application `ld_noise`:

-	Application `ld_slip`:

-	Application `sm0`: old test.

-	Application `sm1`: useful for running experiments. Reads scans
	from two different files, and outputs statistics.

-	Application `sm2`: standard scan-matching.


### Unstable things: scripts ###

In the `scripts/` directory you can find:

-	Script `fig2pics.rb`: used for converting FIG files to PDF.
	It has many more options than [`fig2dev`][fig2dev] (that is being used internally), 
	including the ability to use a LaTeX preamble and to change
	the resulting bounding box.
	
-	Script `json2matlab.rb`: converts a JSON object in a Matlab scripts.
	This is the holy grail of data exchange.
	
	Warning: at the moment, this script relies on some patches to
	the Ruby JSON library. Without them, it is limited to only
	1 JSON object in each file.

-	Script `create_video.rb`: displays the scan-matching process.
	This reads the journal files written by applications `sm1` and `sm2`.


### Unstable things: Ruby and Matlab implementations ###

Unstable things include:

-	Directory `sm_ruby_wrapper/`: a ruby wrapper for the `sm` C library.

-	Directory `rsm/`: a Ruby implementation of the same algorithms used
	in the `sm` library.

-	Directory `matlab/` and `matlab_new/`: once upon a time, the C, Ruby, 
	and Matlab implementation were equivalent, but now they are out of 
	sync. The Matlab scripts are a mess that needs tidying.
	
	Also, I occasionally tried to make sure that the scripts run fine
	in [Octave]. They do, except for the plotting.


[octave]: http://www.octave.org
[cairo]: http://cairographics.org
[json-c]: http://www.json.org
[fig2dev]: http://www.xfig.org
[MIT license]: http://en.wikipedia.org/wiki/MIT_License

