#!/usr/bin/env ruby
require 'rsm'

class Matching 
	attr_accessor :x
	attr_accessor :cov_x
	
	attr_accessor :dx_dy1
	attr_accessor :dx_dy2

	attr_accessor :J1
	attr_accessor :J2
end

def scan_matching(io, scan_matcher)
	include MathUtils
	
	laser_ref = LogReader.shift_laser(io)
	
	count = 0
	
	pose = Vector[0,0,0].col
	cov_pose = Matrix[[0,0,0],[0,0,0],[0,0,0]]
	cov_pose_b = cov_pose
	sigma = 0.01;
	
	history = Array.new
	until io.eof? 
		count+=1 
		laser_sens = LogReader.shift_laser(io)
		min_step = -0.001
		min_theta_step_deg =- 0.01

		u =  pose_diff(laser_sens.odometry, laser_ref.odometry)
	#	puts "u: #{pv(u)}"
		
		if (u[0,1]).nrm2 <= min_step && (u[2].abs <deg2rad(min_theta_step_deg))
			
			# todo: merge readings
			#puts "Skipping (same odometry)"
			next
		end

	#	puts "Ref: #{pv(laser_ref.odometry)}"
	#	puts "New: #{pv(laser_sens.odometry)}"

		icp = scan_matcher.new
		# Write log of the icp operation
	#	icp.journal_open("icp_sm.rb-sm#{count}.txt")

		icp.params[:max_angular_correction_deg]= 10
		icp.params[:max_linear_correction]=  2
		icp.params[:laser_ref] = laser_ref;
		icp.params[:laser_sens] = laser_sens;
		icp.params[:firstGuess] = u
		icp.params[:max_iterations] = 20


		m = Matching.new
		
		m.x, m.dx_dy1, m.dx_dy2 = icp.scan_matching
		m.J1 = J1(pose,m.x);
		m.J2 = J2(pose,m.x);
		
		
		pose_new = oplus(pose, m.x);
		m.cov_x = (sigma*sigma) * m.dx_dy1 * (m.dx_dy1.trans) +
		          (sigma*sigma) * m.dx_dy2 * (m.dx_dy2.trans);
		
		
		cov_pose_new = m.J1 * cov_pose * m.J1.trans +  m.J2 * m.cov_x * m.J2.trans

		if true
			if history.empty?
				cov_pose_b_new = m.J1 * cov_pose_b * m.J1.trans +  m.J2 * m.cov_x * m.J2.trans
			else
				d = (sigma*sigma) * m.J1 * history[-1].J2 * history[-1].dx_dy2 * m.dx_dy1.trans * m.J2.trans;
				puts " D = \nd"
				cov_pose_b_new = m.J1 * cov_pose_b * m.J1.trans +  m.J2 * m.cov_x * m.J2.trans + d + d.trans
			end
		end
		
		
#		puts "j1 =\n #{j1}"
#		puts "j2 =\n #{j2}"
#		puts "j1*j1' =\n #{j1*j1.trans}"
#		puts "j2*j2' =\n #{j2*j2.trans}"
		puts "PPP pose = #{pv(pose)} \ncov  = #{pm(cov_pose_new)}"
		puts "cov2 = #{pm(cov_pose_b_new)}"
		puts "cov = \n#{cov_pose_new}"
		puts "cov2= \n#{cov_pose_b_new}"
		puts "cov-cov2 = \n#{cov_pose_new-cov_pose_b_new}"
		
		history.push m
		pose = pose_new;
		cov_pose = cov_pose_new;
		cov_pose_b = cov_pose_b_new;
		laser_ref = laser_sens;
	end
end

# Read from standard input if no arguments are passed
io = ARGV.size>0 ? File.open(ARGV[0]) : $stdin 

scan_matching(io)

