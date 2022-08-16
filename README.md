For more information about the C(anonical) Scan Matcher, see the webpage: http://purl.org/censi/2007/csm .

This is the "master" branch of CSM, which uses GSL.

# Branching

- Main, development branch: [`master`][branch]. Note this uses `GSL`, which is licensed as GPL as of 2022/08.
- Another development branch: [`master_eigen`][branch]. This uses license permissible library `eigen`, instead of GSL.
   - As of 2022/08/15, this branch is behind `master`. Cherry-picking effort for the newer changes that are only merged into `master` is appreciated.
- Deprecated: [``csm_eigen``][branch], which uses the ``eigen`` library. This branch is the work of people working at U. Freiburg and Kuka, including Christoph Sprunk and Rainer Kuemmerle. See also [csm#33#issuecomment](https://github.com/AndreaCensi/csm/issues/33#issuecomment-1186263053).

Binary install (via ROS)
------------------------------

(November 2015) Now you can install binary on Ubuntu (via ROS). As of today limited to Ubuntu Saucy and Trusty. To do so:

 1. Add ROS repository to your Ubuntu's download site (For detail, see [ROS wiki](http://wiki.ros.org/indigo/Installation/Ubuntu)):

 ```
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
sudo apt-key adv --keyserver hkp://pool.sks-keyservers.net --recv-key 0xB01FA116
sudo apt-get update
```

 2. Install CSM. 

 ```
sudo apt-get install ros-indigo-csm
```

The package name contains "ROS" specific info, but you can use this as a standalone CSM library. It goes into these directory:

 ```
/opt/ros/indigo/include/csm
/opt/ros/indigo/lib/libcsm-static.a
/opt/ros/indigo/lib/libcsm.so
```
