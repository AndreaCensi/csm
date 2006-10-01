swig -ruby icpc.i
ruby extconf.rb
make clean
make
make install
echo "puts require('icpc')" | ruby
