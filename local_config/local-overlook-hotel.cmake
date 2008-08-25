
# Melania is a 1.4GhZ PPC Powerbook

#SET(GSL_CONFIG_PREFER_PATH /Users/andrea/csm/csm/deploy/bin)
#SET(GSL_CONFIG /Users/andrea/csm/csm/deploy/bin/gsl-config)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-double")
SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-double")

# 
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstrict-aliasing")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ggdb")


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funroll-loops")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-loops=16")

