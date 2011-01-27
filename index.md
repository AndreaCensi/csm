The C(anonical) Scan Matcher
============================

![PL-ICP](sm_plicp_zoom_crop.gif){:style="float: right; margin:1em;"}


### News ###

* Fall 2010: [CSM has been integrated and packaged][stack] for [ROS] by [Ivan Dryanovski][ivan].
    

### Download ###

* CSM can be download from GitHub (.zip package available) at the url:

  [http://github.com/AndreaCensi/csm](http://github.com/AndreaCensi/csm)

[stack]: http://www.ros.org/wiki/canonical_scan_matcher
[ivan]: http://robotics.ccny.cuny.edu/blog/People/Dryanovski
[ROS]: http://www.ros.org/
 

### Documentation / getting started ###

Please see the manual contained in "csm_manual.pdf". See below for a quick description.


**What is this.** I created this package:

- To have a well-documented reference implementation of [PL-ICP](http://purl.org/censi/2007/plicp). If you are only interested in the core algorithm of PL-ICP, a [separate concise implementation in C/Matlab/Ruby](http://purl.org/censi/2007/gpc) is available.

- To have a **trustworthy** scan matcher to be used in the experiments for some papers on [ICP covariance](http://purl.org/censi/2006/icpcov), [the Cramer-Rao bound for range finders](http://purl.org/censi/2006/accuracy), and [robot calibration](http://purl.org/censi/2007/calib).  For batch experiments, it's also useful that it's pretty fast.

- To have a collection of utilies for command line (UNIX-style) manipulation of laser data,
  and creating [beautiful maps][map-example] and animations.

The package contains also a Ruby wrapper for the C library, and additional Ruby and a Matlab implementations of the same algorithm. These are not as usable or documented as the C version.

**What it is NOT**: Note that this is not a full-featured SLAM solution: this only does pairwise scan-matching between scans (but it's really good at it!).
If you are looking for a more complete SLAM solution, please see the projects listed in the [OpenSLAM](http://www.openslam.org) page; in particular you can have a look at [GMapping]. 
Many pointers to other SLAM software can be found on the pages of the Euron SLAM summer schools: 
[2002 (Stockholm)](http://www.cas.kth.se/SLAM/),
[2004 (Toulouse)](http://www.laas.fr/SLAM/),
[2006 (Oxford)](http://www.robots.ox.ac.uk/~SSS06/Website/index.html).
Other related projects are [Carmen] and [Stage].

[map-example]: ../plicp/laserazosSM3.log.pdf

[gmapping]: http://www.openslam.org/gmapping.html
[carmen]: http://carmen.sourceforge.net/
[stage]: http://playerstage.sourceforge.net/



-----------

Please link to this page using the url <http://purl.org/censi/2007/csm>.