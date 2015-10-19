## This CMakeLists.txt file exists only for building as ROS 3rd-party package. Building for other purpose is not tested. See https://github.com/AndreaCensi/csm/pull/10
cmake_minimum_required(VERSION 2.8)
project(csm)

INSTALL(FILES package.xml DESTINATION share/csm)
