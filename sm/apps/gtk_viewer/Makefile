
all_targets=gtk_viewer1

all: $(all_targets) 

GOO_FLAGS=-I/usr/local/include/goocanvas-1.0/ -lgoocanvas
GTK_FLAGS=`pkg-config --libs --cflags gtk+-2.0 gthread-2.0`
CSM_FLAGS=`pkg-config --libs --cflags csm`

FLAGS=$(GTK_FLAGS) $(GOO_FLAGS) $(CSM_FLAGS) -Wno-long-double -O3
%: %.c goo_laser_data.c
	gcc -g $(FLAGS) -o $@ $^


clean:
	rm $(all_targets)