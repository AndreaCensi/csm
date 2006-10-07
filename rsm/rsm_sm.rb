#!/usr/bin/env ruby
require 'logreader'

def scan_matching(io, klass)
	include MathUtils
	
	laser_ref = LogReader.shift_laser(io)
	
	count = 0
	until io.eof?
		
		laser_sens = LogReader.shift_laser(io)
		min_step = 0.2
		min_theta_step_deg = 5;

		u =  pose_diff(laser_sens.odometry, laser_ref.odometry)
		
		if (u[0,1]).nrm2 <= min_step && (u[2].abs <deg2rad(min_theta_step_deg))
			
			# todo: merge readings
			#puts "Skipping (same odometry)"
			next
		end

#		puts "Ref: #{pv(laser_ref.odometry)}"
#		puts "New: #{pv(laser_sens.odometry)}"

		count += 1
		
		sm = klass.new
		# Write log of the icp operation
		sm.journal_open("icp_sm.#{sm.name}.#{count}.txt")
		sm.params = standard_parameters
		sm.params[:maxAngularCorrectionDeg]= 60
		sm.params[:maxLinearCorrection]=  2
		sm.params[:laser_ref] = laser_ref;
		sm.params[:laser_sens] = laser_sens;
		sm.params[:maxIterations] = 20
		sm.params[:firstGuess] = u

		#		sm.params[:laser_sens] = laser_ref;
		#		sm.params[:firstGuess] = GSL::Vector.alloc(0.2,0.2,deg2rad(30))

		res = sm.scan_matching
		
		puts "u: #{pv(u)}"
		puts "x: #{pv(res[:x])}"
		
		laser_ref = laser_sens;
	end
end

scan_matcher = eval ARGV.shift

# Read from standard input if no arguments are passed
io = ARGV.size>0 ? File.open(ARGV[0]) : $stdin 

scan_matching(io, scan_matcher)

