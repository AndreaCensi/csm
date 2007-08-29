# sorchetta is an Intel Duo 2 with 2.0 Ghz for core

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=prescott -fomit-frame-pointer -O3 -msse3 -mfpmath=sse -pipe")
