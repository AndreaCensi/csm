require 'sm'
require 'structures'
require 'journal'
	
# pass parameters 
def params2method(params, ob)
	params.each { |param, value|
		method = "#{param}="
		if ob.methods.include? method
			ob.__send__(method, value) 
		else
			#puts "Structure does not have method #{method}"
		end
	}
end

def put_params_in_c_structures(params)
# pass all parameters to extension library
	laser_ref  = params[:laser_ref]
	laser_sens = params[:laser_sens]

	Sm::rb_sm_l_nrays(0, laser_ref.nrays);
	Sm::rb_sm_l_min_theta(0, laser_ref.min_theta);
	Sm::rb_sm_l_max_theta(0, laser_ref.max_theta);
	laser_ref.points.each_index do |j|
		Sm::rb_sm_l_ray(0, j, laser_ref.points[j].theta, laser_ref.points[j].reading)
	end

	Sm::rb_sm_l_nrays(1, laser_sens.nrays);
	Sm::rb_sm_l_min_theta(1, laser_sens.min_theta);
	Sm::rb_sm_l_max_theta(1, laser_sens.max_theta);
	laser_sens.points.each_index do |i|
		Sm::rb_sm_l_ray(1, i, laser_sens.points[i].theta, laser_sens.points[i].reading)
	end
	u=params[:firstGuess];
	rb_sm_odometry(u[0],u[1],u[2]);

	params2method(params, Sm::rb_sm_params)
end

def get_result_from_c_structures()
	res = Hash.new 
	x = rb_sm_get_x();
	res[:x] = Vector[x[0],x[1],x[2]]
	res[:iterations] = Sm::rb_sm_result.iterations;
	res[:error] = Sm::rb_sm_result.error
end

class ICPC
	include MathUtils	
	include Math
	include Sm
	
	attr_accessor :params
	
	def initialize
		@params = standard_parameters
	end
	
	def name 
		"ICPC"
	end

	def scan_matching
		
	
		put_params_in_c_structures(params)
	
		rb_sm_icp()

		res = get_result_from_c_structures()
			
		rb_sm_cleanup
		
		res
	end
	
	
	def journal_open(filename)
		rb_sm_init_journal(filename)
	end
end