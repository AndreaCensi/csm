require 'icpc'

require 'structures'
require 'journal'
	
class ICPC
	include MathUtils	
	include Math
	include Icpc
	
	attr_accessor :params
	
	def initialize
		@params = Hash.new
		standard_parameters
	end

	def standard_parameters
		@params[:maxAngularCorrectionDeg]= 30
		@params[:maxLinearCorrection]=    0.5
		@params[:maxCorrespondenceDist]=   2
		@params[:maxIterations]=           40
		@params[:firstGuess]=         Vector.alloc(0,0,0)
		@params[:epsilon_xy]=  0.001
		@params[:epsilon_theta]=   deg2rad(0.01)
		@params[:sigma]=           0.01
	end
	
	
	def scan_matching
		
		# pass all parameters to extension library
		laser_ref  = params[:laser_ref]
		laser_sens = params[:laser_sens]
		
		icpc_l_nrays(0, laser_ref.nrays);
		icpc_l_min_theta(0, laser_ref.min_theta);
		icpc_l_max_theta(0, laser_ref.max_theta);
		laser_ref.points.each_index do |j|
			icpc_l_ray(0, j, laser_ref.points[j].theta, laser_ref.points[j].reading)
		end

		icpc_l_nrays(1, laser_sens.nrays);
		icpc_l_min_theta(1, laser_sens.min_theta);
		icpc_l_max_theta(1, laser_sens.max_theta);
		laser_sens.points.each_index do |i|
			icpc_l_ray(1, i, laser_sens.points[i].theta, laser_sens.points[i].reading)
		end
	
		# pass parameters 
		Icpc::icpc_params.maxAngularCorrectionDeg = params[:maxAngularCorrectionDeg]
		Icpc::icpc_params.maxLinearCorrection     = params[:maxLinearCorrection]
		Icpc::icpc_params.maxIterations           = params[:maxIterations]
		Icpc::icpc_params.epsilon_xy              = params[:epsilon_xy]
		Icpc::icpc_params.epsilon_theta           = params[:epsilon_theta]
		Icpc::icpc_params.maxCorrespondenceDist   = params[:maxCorrespondenceDist]
		
#		icpc_odometry(double x, double y, double theta);

		icpc_go()
		
	end
	
	
	def journal_open(filename)
		icpc_init_journal(filename)
	end
end