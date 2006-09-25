require 'gsl'

module GSL
	def deg2rad(d); d * Math::PI / 180; end
	def rad2deg(d); d / Math::PI * 180; end
end

class ICP < Hash
	include GSL
	attr_accessor :params
	
	def initialize(ld1, ld2)
		@params = Hash.new
		standard_parameters
	end

	def standard_parameters
		param :maxAngularCorrectionDeg, 105
		param :maxAngularCorrectionDeg, 105
		param :maxCorrespondenceDist,   4
		param :maxLinearCorrection,     2
		param :maxIterations,           40
		param :firstGuess,         Vector.alloc(0,0,0)
		param :interactive,  false
		param :epsilon_xy,  0.0001
		param :epsilon_theta,  deg2rad(0.001)
		param :sigma,           0.01
		param :do_covariance,  false
	end
	
	def param(symbol, value)
		@params[symbol] = value
	end
end

class ICP
	class Correspondence
		attr_accessor :p
		attr_accessor :q
		attr_accessor :i_k
		attr_accessor :j1_k
		attr_accessor :j2_k
		attr_accessor :valid
	end
	
	def scan_matching
		x_old = params[:firstGuess]
		for iteration in 1..params[:maxIterations]
			puts "Iteration #{iteration}"
		
			find_correspondences(x_old);
		
			x_new = compute_next_estimate

			delta = x_old-x_new;
	
			puts "x_new = #{x_new}"
			puts "delta = #{delta}"
		
			if delta[0,1].nrm2 < params[:epsilon_xy] &&
			   delta[2].abs < params[:epsilon_theta]
				break;
			end
			
			x_old = x_new;
		end
		
	end
	
	def find_correspondences(x_old);
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];
		
	end
	
	def compute_next_estimate
		Vector.alloc(0,0,0)
	end
	
	
end
