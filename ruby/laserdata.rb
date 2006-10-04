require 'mathutils'

class Event
	attr_accessor :timestamp
	attr_accessor :ipc_timestamp
	attr_accessor :hostname
end

class LaserData < Event
	include Math
	attr_accessor :odometry
	attr_accessor :estimate
	attr_accessor :points
	attr_accessor :nrays
	attr_accessor :min_reading
	attr_accessor :max_reading
	attr_accessor :min_theta
	attr_accessor :max_theta
end

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


def standard_parameters
	p = Hash.new
	p[:maxAngularCorrectionDeg]= 30
	p[:maxLinearCorrection]=    0.5
	p[:maxCorrespondenceDist]=   2
	p[:maxIterations]=           40
#	p[:firstGuess]=         Vector.alloc(0,0,0)
	p[:epsilon_xy]=  0.0001
	p[:epsilon_theta]=   0.0001
	p[:sigma]=           0.01
	p
end


require 'laserdata_carmen'
