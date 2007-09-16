
# linuxserv is a 4 x Xeon 1.8Gh

#icc
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=pentium4 ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -wd981 ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -wd1418 -wd1419")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fp-model fast ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall ")





