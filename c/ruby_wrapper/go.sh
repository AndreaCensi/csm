swig -ruby icpc.i
ruby extconf.rb --with-opt-dir=../../deploy
make clean
make
make install
echo "puts require('icpc')" | ruby
