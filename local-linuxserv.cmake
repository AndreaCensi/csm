
# linuxserv is a 4 x Xeon 1.8Gh

#gcc
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -O3")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=pentium4 -msse2 -mfpmath=sse -mmmx -funroll-loops")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftree-vectorize -ftree-vectorizer-verbose=5")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")

