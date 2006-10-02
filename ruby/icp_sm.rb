#!/usr/bin/env ruby
require 'logreader'

require 'icp'
require 'icpc_wrap'

def scan_matching(io)
	include MathUtils
	

	laser_ref = LogReader.shift_laser(io)
	
	count = 0
	until io.eof?
		
		laser_sens = LogReader.shift_laser(io)
		min_step = 0.2
		min_theta_step_deg = 5;

		u =  pose_diff(laser_sens.odometry, laser_ref.odometry)
		puts "u: #{pv(u)}"
		
		if (u[0,1]).nrm2 <= min_step && (u[2].abs <deg2rad(min_theta_step_deg))
			
			# todo: merge readings
			#puts "Skipping (same odometry)"
			next
		end

		puts "Ref: #{pv(laser_ref.odometry)}"
		puts "New: #{pv(laser_sens.odometry)}"


		icp = ICP.new
		# Write log of the icp operation
		icp.journal_open("icp_sm.rb-sm#{count+=1}.txt")

		icp.params[:maxAngularCorrectionDeg]= 10
		icp.params[:maxLinearCorrection]=  2
		icp.params[:laser_ref] = laser_ref;
		icp.params[:laser_sens] = laser_sens;
#		icp.params[:firstGuess] = u
		icp.params[:maxIterations] = 20
#		icp.params[:laser_sens] = laser_ref;
#		icp.params[:firstGuess] = GSL::Vector.alloc(0.2,0.2,0)
	
#		icp.scan_matching

		if true
			require 'icpc_wrap'
			
			icpc = ICPC.new
			# Write log of the icp operation
			icpc.journal_open("icp_sm.rb-c#{count}.txt")

			icpc.params = icp.params
			
			icpc.scan_matching
		end




		laser_ref = laser_sens;
	end
end

# Read from standard input if no arguments are passed
io = ARGV.size>0 ? File.open(ARGV[0]) : $stdin 

scan_matching(io)

