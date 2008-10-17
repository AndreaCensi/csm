require 'mkmf'

GSL_CONFIG = "gsl-config"

# Taken from rb-gsl
def gsl_config()
  print("checking gsl cflags... ")
  IO.popen("#{GSL_CONFIG} --cflags") do |f|
    cflags = f.gets.chomp
    puts(cflags)
    $CFLAGS += " " + cflags
  end
   
  IO.popen("#{GSL_CONFIG} --libs") do |f|
    libs = f.gets.chomp
    dir_config("cblas")
    dir_config("atlas")
    if have_library("cblas") and have_library("atlas")
      libs.gsub!("-lgslcblas", "-lcblas -latlas")
      $LOCAL_LIBS += " " + libs.gsub(" -lgslcblas", "")
      print("checking gsl libs... ")
      puts(libs)
    else
      print("checking gsl libs... ")
      puts(libs)
      $LOCAL_LIBS += " " + libs
    end
  end

end

def crash(str)
  print " extconf failure: #{str}\n"
  exit 1
end


if (not have_library('csm')) #or (not find_header('icp.h','/usr/local/include'))
	$stderr.puts "Error: not having library 'csm'"
	exit
else
	$LOCAL_LIBS += ' -lcsm'
end

$CPPFLAGS += " -Wall -W -Wmissing-prototypes -Wconversion "
$CPPFLAGS += " -Wunreachable-code "
$CPPFLAGS += " -DRUBY"
gsl_config();

srcs = %w(rb_sm sm_wrap) 
$objs = srcs.collect{|i| i+".o"}

create_makefile('sm')
if false
File.open("Makefile","a") do |f|
	f.puts <<-EOF
# Copy other sources from other directory
%.o: ../%.o
	cp $< $@
	
EOF

end end
