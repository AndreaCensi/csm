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
	

	attr_accessor :odometry
	attr_accessor :estimate

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
		
		@odometry    = Vector[GSL::NAN,GSL::NAN,GSL::NAN].col
		@estimate    = Vector[GSL::NAN,GSL::NAN,GSL::NAN].col
		@p = []; @corr = [];
		for i in 0..nrays-1
			@p[i] = Vector[GSL::NAN,GSL::NAN].col
			@corr[i] = Correspondence.new;
			@corr[i].valid = false;
			@corr[i].j1 = -1;
			@corr[i].j2 = -1;
		end
		
		# jump tables
		@up_bigger = [nil]*nrays
		@up_smaller = [nil]*nrays
		@down_bigger = [nil]*nrays
		@down_smaller = [nil]*nrays
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
end

=begin
class LaserPoint 
	include MathUtils
	
	attr_accessor :reading
	attr_accessor :intensity
	attr_accessor :theta
	
	# Estimated alpha (relative to robot)
	attr_accessor :alpha
	attr_accessor :alpha_valid
	# Covariance of estimated alpha (relative to robot)
	attr_accessor :cov_alpha

	# Cluster id (integer)
	attr_accessor :cluster

	attr_accessor :valid
	
	def alpha_valid?; @alpha_valid end
	def valid?; @valid end
	def v; MathUtils.vers(theta) end
	def cartesian; MathUtils.vers(theta)*reading; end
end
=end


def standard_parameters
	p = Hash.new
	p[:maxAngularCorrectionDeg]= 30
	p[:maxLinearCorrection]=    0.5
	p[:maxCorrespondenceDist]=   2
	p[:maxIterations]=           40
	p[:epsilon_xy]=  0.0001
	p[:epsilon_theta]=   0.0001
	p[:sigma]=           0.01
	p[:restart]=         1
	p[:restart_dt]=      0.1
	p[:restart_dtheta]=     deg2rad(1.5)
	
	p[:clusteringThreshold] = 0.05
	p[:orientationNeighbourhood] = 3
	
	p[:doAlphaTest] = 1
	p[:doAlphaTest_thresholdDeg]=20
	
	p[:outliers_maxPerc] = 0.95;
	
	p
end

require 'rsm_laserdata_ops'
require 'rsm_laserdata_carmen'
