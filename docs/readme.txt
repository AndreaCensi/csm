
The C(anonical) Scan Matcher 
================================

* TOC
{:toc}

## Introduction ##

I created this package:

- To have a well-documented reference implementation of [PL-ICP](http://purl.org/censi/2007/plicp). If you are only interested in the core algorithm of PL-ICP, a [separate concise implementation in C/Matlab/Ruby](http://purl.org/censi/2007/plicp) is available.

- To have a **trustworthy** scan matcher to be used in the experiments for some papers on [ICP covariance](http://purl.org/censi/2006/icpcov), [the Cramer-Rao bound for range finders](http://purl.org/censi/2006/accuracy), and [robot calibration](http://purl.org/censi/2007/calib).  For batch experiments, it's also useful that it's pretty fast.

- To have a collection of utilies for command line (UNIX-style) manipulation of laser data, and creating beautiful maps and animations.

## Content of this package ##

The core content is the C scan matching library which is quite polished, but this package contains a lot of software, only some of that in an usable state.
In general, I am not ashamed of the prototypical code I write.

### Stable things: C scan matching library ###

The directory `sm/csm` contains a scan matcher written in C, plus 
associated tools and apps. This is stable and reasonably bug-free.

There are many libraries in the `sm/lib` directory:

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

-	Application `sm2`: standard scan-matching. Reads a log, runs ICP, and writes the scan-matched output. Input can be both Carmen and JSON.

-	Application `sm3`: like sm2, but instead of actual output it measures the performance. This is the application that produced the stats found in the paper submitted to ICRA'08.

-	Application `sm1`: useful for running experiments. Reads scans
	from two different files, and outputs statistics.

Visualization apps:

-	Application `log2pdf`: converts a laser log to a PDF map.
	To build this application, it is needed to install the [Cairo] graphics
	library.

-	Application `sm_animate`: creates an animation for the ICP process, displaying the correspondences, etc. This application reads the output created by `sm2` with the `-file_jj` option. To build this application, it is needed to install the [Cairo] graphics
library.

Miscellaneous Unix-style processing for laser data:

-	Application `carmen2json`: converts a Carmen log to the JSON format.

-	Application `ld_fisher`: computes the Fisher's information matrix. See <http://purl.org/censi/2006/accuracy> for details.

-	Application `json_extract`: extract the n-th object from a JSON stream.

-	Application `ld_slip`: adds some noise to the odometry field.

-	Application `ld_smooth`: smooths the readings data.

-	Application `ld_noise`: adds sensor noise.

-	Application `ld_cluster_curv`: clusterize the rays based on the analysis of the curvature.

-	Application `ld_linearize`: fits a line to each cluster (data must have been previously clustered, for example by `ld_cluster_curv`).


GUI apps:

-   `apps/gtk_viewer` contains the prototype of a viewer using GTK. It does not work yet.

### Unstable things: scripts ###

In the `scripts/` directory you can find:

-	Script `json2matlab.rb`: converts a JSON object in a Matlab scripts.
	This is the holy grail of data exchange.
	
	Warning: at the moment, this script relies on some patches to
	the Ruby JSON library. Without them, it is limited to only
	1 JSON object in each file.

-	Script `fig2pics.rb`: used for converting FIG files to PDF.
	It has many more options than [`fig2dev`][fig2dev] (that is being used internally), 
	including the ability to use a LaTeX preamble and to change
	the resulting bounding box.

-	Script `create_video.rb`: displays the scan-matching process.
	This reads the journal files written by applications `sm1` and `sm2`.
	**Made obsolete by `sm_animate`**

### Unstable things: Ruby and Matlab implementations ###

Unstable things include:

-	Directory `sm_ruby_wrapper/`: a ruby wrapper for the `sm` C library. This wrapper is used for running some of the experiments. It is not documented and it needs tidying a little.

-	Directory `rsm/`: a Ruby implementation of the same algorithms used
	in the `sm` library. Some times ago, the C and Matlab implementation
	were perfectly in sync. Now they differ a little. However, in the future
	I will try to get them back in sync, as the only way of having a good
	chance of making a bug-free implementation, is to make it twice.

-	Directory `matlab/` and `matlab_new/`. The Matlab scripts are a mess that needs tidying. There's a lot in there. They are kept here because they are used for creating some of the figures in the submitted papers. Also, the first PLICP implementation was written in Matlab and is buried there, somewhere.
	
	Also, I occasionally tried to make sure that the scripts run fine
	in [Octave]. They do, except for the plotting.


[octave]: http://www.octave.org
[cairo]: http://cairographics.org
[json-c]: http://www.json.org
[fig2dev]: http://www.xfig.org
[MIT license]: http://en.wikipedia.org/wiki/MIT_License

