#begin
	require 'sm'
	require 'rsm'
# rescue => ex
# 	puts ex, ex.backtrace
# end

module Sm
	
# pass parameters 
def Sm.params2method(params, ob)
	params.keys.map{|x|x.to_s}.sort.map{|x|x.to_sym}.
	each { |param| value = params[param]
		method = "#{param}="
		if ob.methods.include? method
			old_value = ob.__send__(param.to_s)
#			ob.__send__(method, value)
			if old_value  != value
				$stderr.puts "Setting #{method} #{value} (#{old_value})" 
		
				if 0 == rb_sm_set_configuration(param.to_s, value.to_s);
					raise "Error setting #{param.inspect}"
				end
			end
		else
			raise "Structure does not have method #{method}"
		end
	}
end

def Sm.put_params_in_c_structures(params)
# pass all parameters to extension library
	laser_ref  = params[:laser_ref]
	laser_sens = params[:laser_sens]

	c_jo = json_parse(laser_ref.to_json)
	c_ld = json_to_ld(c_jo); jo_free(c_jo);
	rb_sm_params.laser_ref = c_ld;
	
	c_jo = json_parse(laser_sens.to_json)
	c_ld = json_to_ld(c_jo); jo_free(c_jo);
	rb_sm_params.laser_sens = c_ld;

#	ld_free(c_ld);
#	ld_free(c_ld);
	
	u=params[:firstGuess];
	rb_sm_odometry(u[0],u[1],u[2]);

	remove = [:laser_ref, :firstGuess, :laser_sens]
	filtered = params.delete_if {|k,v| remove.include? k }
	Sm.params2method(params, Sm::rb_sm_params)
end

def Sm.get_result_from_c_structures()
	
	ld_free(rb_sm_params.laser_sens);
	ld_free(rb_sm_params.laser_ref);
	
	
	res = JSON.parse(rb_result_to_json)

	res2 = {}
	res.keys.each do |k|
		res2[k.to_sym] = res[k]
	end
	res = res2
	
	p res
	res[:valid] = res[:valid] == 1
	if res[:valid]
		res[:x] = Vector[*res[:x]].col
	end
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