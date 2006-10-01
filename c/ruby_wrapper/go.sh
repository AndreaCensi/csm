swig -ruby icpc.i
ruby extconf.rb
make
make install
echo "puts require('icpc')" | ruby
