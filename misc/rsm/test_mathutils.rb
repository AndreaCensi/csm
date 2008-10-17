require 'rsm_mathutils'

include MathUtils 
include GSL

pose1 = Vector[1, 2, 3]
pose2 = Vector[1.5, 2.1, 2.4]

puts "pose1\t #{pv(pose1)}"
puts "pose2\t #{pv(pose2)}"
diff = pose_diff(pose2, pose1)
puts "diff\t #{pv(diff)}"
pose2b = oplus(pose1, diff)
puts "pose2b\t #{pv(pose2b)}"
