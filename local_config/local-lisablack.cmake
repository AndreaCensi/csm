
# Lisablack is a Pentium 3


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fomit-frame-pointer -O3")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=pentium3 -msse -O3 -pipe")


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
