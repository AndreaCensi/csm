require 'rsm_mathutils'

class Correspondence
	attr_accessor :valid
	attr_accessor :j1
	attr_accessor :j2
end

class LaserData
	include Math
	
	attr_accessor :nrays
	attr_accessor :min_reading
	attr_accessor :max_reading
	attr_accessor :min_theta
	attr_accessor :max_theta
	
	attr_accessor :readings
	attr_accessor :valid
	attr_accessor :theta
	
	attr_reader :alpha
	attr_accessor :alpha_valid
	attr_accessor :cov_alpha

	attr_accessor :cluster
	attr_accessor :p
	
	attr_accessor :cov_readings

	attr_accessor :odometry
	attr_accessor :estimate
	attr_accessor :true_pose

	attr_accessor :timestamp
	attr_accessor :ipc_timestamp
	attr_accessor :hostname
	
	attr_accessor :up_bigger
	attr_accessor :up_smaller
	attr_accessor :down_bigger
	attr_accessor :down_smaller

	attr_accessor :corr
	
	def initialize(nrays)
		@nrays      = nrays
		@valid       = [false  ] * nrays
		@readings    = [GSL::NAN] * nrays
		@theta       = [GSL::NAN] * nrays
		@alpha_valid = [false  ] * nrays
		@alpha       = [GSL::NAN] * nrays
		@cov_alpha   = [GSL::NAN] * nrays
		@cluster     = [-1] * nrays
		@cov_readings = [GSL::NAN]* nrays
		
		@odometry    = [GSL::NAN,GSL::NAN,GSL::NAN].to_gv.col
		@estimate    = [GSL::NAN,GSL::NAN,GSL::NAN].to_gv.col
		@true_pose   = [GSL::NAN,GSL::NAN,GSL::NAN].to_gv.col
		
		@p = []; @corr = [];
		for i in 0..nrays-1
			@p[i] = Vector[GSL::NAN,GSL::NAN].col
			@corr[i] = Correspondence.new;
			@corr[i].valid = false
			@corr[i].j1 = -1
			@corr[i].j2 = -1
		end
		
		# jump tables
		@up_bigger = [nil]*nrays
		@up_smaller = [nil]*nrays
		@down_bigger = [nil]*nrays
		@down_smaller = [nil]*nrays
		
		# min_theta ? 
	end	
	
	def alpha_valid?(i); i>=0 && i<nrays && @alpha_valid[i] end
	def valid?(i); i>=0 && i<nrays && @valid[i] end
	def v(i); MathUtils.vers(theta[i]) end
#	def cartesian; MathUtils.vers(theta[i])*reading; end
	
	def set_null_corr(i)
		@corr[i].valid = false
		@corr[i].j1 = -1
		@corr[i].j2 = -1 
	end
	
	def next_valid_down(i); next_valid(i,-1) end
	def next_valid_up  (i); next_valid(i,+1) end
	def next_valid(i, direction)
		i += direction
		while i>=0 && i<nrays
			return i if valid? i
			i += direction
		end
		nil
	end
	
	def correspondences_hash
		indexes = @corr.map{|c| c.j1}
		indexes.hash
	end
end

def standard_parameters
	p = Hash.new
	p[:max_angular_correction_deg]= 90
	p[:max_linear_correction]=    2
	p[:max_correspondence_dist]=   2
	p[:max_iterations]=           40
	p[:epsilon_xy]=  0.0001
	p[:epsilon_theta]=   0.0001
	p[:sigma]=           0.01
	p[:restart]=         1
	p[:restart_threshold_mean_error] = 3.0 / 300.0
	p[:restart_dt]=      0.1
	p[:restart_dtheta]=  0.026;#  1.5 * 3.14 /180
	
	p[:clustering_threshold] = 0.05
	p[:orientation_neighbourhood] = 3
	p[:use_corr_tricks] = 1;
	p[:do_compute_covariance] = 0;
		
	p[:do_alpha_test] = 0
	p[:do_alpha_test_thresholdDeg]=20
	
	p[:outliers_maxPerc] = 0.95;
	p[:outliers_adaptive_order] = 0.7; 
	p[:outliers_adaptive_mult] = 2; 
	
	p[:do_visibility_test] = 0
	p
end


def tro_parameters
	p = Hash.new
	p[:max_angular_correction_deg]= 30
	p[:max_linear_correction]=    0.5
	p[:max_correspondence_dist]=   2
	p[:max_iterations]=           40
	p[:epsilon_xy]=  0.0001
	p[:epsilon_theta]=   0.0001
	p[:sigma]=           0.01
	p[:restart]=         1
	p[:restart_threshold_mean_error] = 3.0 / 300.0
	p[:restart_dt]=      0.01
	p[:restart_dtheta]=  0.027 #1.5 * 3.14 /180
	
	p[:clustering_threshold] = 0.05
	p[:orientation_neighbourhood] = 3
	p[:use_corr_tricks] = 1;
	p[:do_compute_covariance] = 0;
		
	p[:do_alpha_test] = 0
	p[:do_alpha_test_thresholdDeg]=20
	
	p[:outliers_maxPerc] = 0.95;
	p[:outliers_adaptive_order] = 0.7; 
	p[:outliers_adaptive_mult] = 2; 
	
	p[:do_visibility_test] = 0
	p
end

require 'rsm_laserdata_ops'
require 'rsm_laserdata_carmen'
require 'rsm_laserdata_json'
