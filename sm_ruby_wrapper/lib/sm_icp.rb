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

	s = params[:laser_ref].to_json
	rb_set_laser_ref(s);
	
	s = params[:laser_sens].to_json
	rb_set_laser_sens(s);

	u = params[:firstGuess];
	rb_sm_odometry(u[0],u[1],u[2]);

	remove = [:laser_ref, :firstGuess, :laser_sens]
	filtered = params.clone.delete_if {|k,v| remove.include? k }
	Sm.params2method(filtered, Sm::rb_sm_params)
end

def Sm.get_result_from_c_structures()
	
#	ld_free(rb_sm_params.laser_sens);
#	ld_free(rb_sm_params.laser_ref);
	
	
	res = JSON.parse(rb_result_to_json)

	res2 = {}
	res.keys.each do |k|
		res2[k.to_sym] = res[k]
	end
	res = res2
	
#	p res
	res[:valid] = res[:valid] == 1
	if res[:valid]
		res[:x] = Vector[*res[:x]].col
		res[:avg_error] = res[:error] / res[:nvalid]
	end
	res
	
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