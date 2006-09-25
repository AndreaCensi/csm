require 'structures'

module Point2line
	include Math
	include MathUtils
	include GSL

	class Correspondence
		attr_accessor :p
		attr_accessor :q
		attr_accessor :C
	end

	def general_minimization(correspondences)

	end

	def Point2line.test_gm

		theta = rand*PI/10
		t = [rand, rand]

		corr = Array.new

		10.times do |i| 
			c = Correspondence.new
			c.p = [rand, rand]
			c.q = rot(theta)* Vector.alloc(c.p)+t + Vector.alloc(rand-0.5,rand-0.5);
			alpha = rand
			c.C = vers(alpha)*vers(alpha).trans
			corr[i] = c
		end
	end

end

Point2line.test_gm