require 'GSL'

v = Vector.alloc(1,2,3).col

puts "#{v} is a col"

puts 2 * v  # => a row 
puts v * 2  # => a column
