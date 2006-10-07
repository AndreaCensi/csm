require 'rsm_mathutils'


module Pose 

	def parse_tokens(tokens)
		v = GSL::Vector.alloc(GSL::NAN,GSL::NAN,GSL::NAN).col
		v[0] = tokens.shift.to_f
		v[1] = tokens.shift.to_f
		v[2] = tokens.shift.to_f
		v
	end
	
end


