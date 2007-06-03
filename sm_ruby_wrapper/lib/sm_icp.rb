#begin
	require 'sm'
	require 'rsm'
# rescue => ex
# 	puts ex, ex.backtrace
# end

module Sm
	
# pass parameters 
def Sm.params2method(params, ob)
	params.each { |param, value|
		method = "#{param}="
		if ob.methods.include? method
			ob.__send__(method, value)
	#		puts "Setting #{method} #{value}" 
		else
			puts "Structure does not have method #{method}"
		end
	}
end

def Sm.put_params_in_c_structures(params)
# pass all parameters to extension library
	laser_ref  = params[:laser_ref]
	laser_sens = params[:laser_sens]

#re	rb_set_laser_ref(laser_ref.to_json);
#	rb_set_laser_sens(laser_sens.to_json)
#	puts "\n\nDa ruby: \n\n#{laser_ref.to_json}"
	c_jo = json_parse(laser_ref.to_json)
	c_ld = json_to_ld(c_jo); jo_free(c_jo);
	rb_sm_params.laser_ref = c_ld;
#	ld_free(c_ld);
	
	c_jo = json_parse(laser_ref.to_json)
	c_ld = json_to_ld(c_jo); jo_free(c_jo);
	rb_sm_params.laser_sens = c_ld;
#	ld_free(c_ld);
	

	# Sm::rb_sm_l_nrays(0, laser_ref.nrays);
	# Sm::rb_sm_l_min_theta(0, laser_ref.min_theta);
	# Sm::rb_sm_l_max_theta(0, laser_ref.max_theta);
	# for j in 0..laser_ref.nrays-1
	# 	Sm::rb_sm_l_ray(0, j, laser_ref.valid[j] ? 1 : 0, 
	# 		laser_ref.theta[j], laser_ref.readings[j])
	# end
	# 
	# Sm::rb_sm_l_nrays(1, laser_sens.nrays);
	# Sm::rb_sm_l_min_theta(1, laser_sens.min_theta);
	# Sm::rb_sm_l_max_theta(1, laser_sens.max_theta);
	# for i in 0..laser_sens.nrays-1
	# 	Sm::rb_sm_l_ray(1, i, laser_sens.valid[i] ? 1 : 0  , 
	# 		laser_sens.theta[i], laser_sens.readings[i])
	# end
	u=params[:firstGuess];
	rb_sm_odometry(u[0],u[1],u[2]);

	remove = [:laser_ref, :firstGuess, :laser_sens]
	filtered = params.delete_if {|k,v| remove.include? k }
	Sm.params2method(params, Sm::rb_sm_params)
end

def Sm.get_result_from_c_structures()
	
	res = JSON.parse(rb_result_to_json)

	res2 = {}
	res.keys.each do |k|
		res2[k.to_sym] = res[k]
	end
	res = res2
	
	res[:valid] = res[:valid] == 1
	if res[:valid]
		res[:x] = Vector[*res[:x]]
	end
	p res
	res
	# res = Hash.new 
	# x = rb_sm_get_x();
	# res[:x] = Vector[x[0],x[1],x[2]]
	# res[:iterations] = Sm::rb_sm_result.iterations;
	# res[:error] = Sm::rb_sm_result.error
	# res[:nvalid] = Sm::rb_sm_result.nvalid
	# res
	
end

	class ICPC
		include MathUtils	
		include Math

		attr_accessor :params
	
		def initialize
			@params = standard_parameters
		end
	
		def name 
			"ICPC"
		end

		def scan_matching
			Sm.put_params_in_c_structures(params)
	
			Sm::rb_sm_icp()

			res = Sm.get_result_from_c_structures()
			
			Sm::rb_sm_cleanup
		
			res
		end
	
	
		def journal_open(filename)
			Sm::rb_sm_init_journal(filename)
		end
	end

end