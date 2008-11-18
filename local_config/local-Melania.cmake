
# Melania is a 1.4GhZ PPC Powerbook

SET(GSL_CONFIG_PREFER_PATH /Users/andrea/csm/csm/deploy/bin)
SET(GSL_CONFIG /Users/andrea/csm/csm/deploy/bin/gsl-config)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-double")
SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-double")

# 
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450  -fomit-frame-pointer -O3")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=7450 -mcpu=7450  -O2")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -gfull")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-math-errno  -fno-trapping-math -fno-signaling-nans")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-math-errno -funsafe-math-optimizations -fno-trapping-math -fno-signaling-nans")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wmissing-prototypes  -Wconversion   -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wnested-externs -fshort-enums -fno-common -Winline" )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unreachable-code")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing")

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-loops=16")


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes ")


SET(COMPILE_HSM 0)
