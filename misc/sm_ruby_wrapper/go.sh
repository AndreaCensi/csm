swig -ruby sm.i
ruby extconf.rb --with-opt-dir=../deploy
make clean
make 2>&1  | egrep -v 'never|function|argv|self'
make install
echo "puts require('sm')" | ruby
