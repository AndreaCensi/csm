require 'GSL'

v = Vector.alloc(0,0)

puts "#{v} is a row"

m = Matrix.alloc(2, 3)

puts v * m

# => Ruby/GSL error code 4, index out of range (file vector_source.c, line 29),
#  invalid argument supplied by user (GSL::ERROR::EINVAL)
  
