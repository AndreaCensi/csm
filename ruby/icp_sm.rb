#!/usr/bin/env ruby
require 'logreader'
require 'icp'

def scan_matching
	include MathUtils
	
	log = LogReader.read_log($stdin)

	def next_ld(log)
#	do
#		e = log.shift
#		if e.kind_of? LaserData then e
#	until e.kind_of?(LaserData)
	log.shift
	end

	puts "Start processing: #{log.size}"

	ld_ref = next_ld(log)
	until log.empty?
	ld_new = next_ld(log)
	min_step = -0.2

	if (ld_new.odometry[0,1]-ld_ref.odometry[0,1]).nrm2 <= min_step 
		# todo: merge readings
		puts "Skipping (same odometry)"
		next
	end

	puts "Ref: #{pv(ld_ref.odometry)}"
	puts "New: #{pv(ld_new.odometry)}"
	u =  pose_diff(ld_new.odometry, ld_ref.odometry)
	puts "u: #{pv(u)}"


	icp = ICP.new

	icp.params[:maxAngularCorrectionDeg]= 10
	icp.params[:maxLinearCorrection]=  0.2

	icp.params[:laser_ref] = ld_ref;
	icp.params[:laser_sens] = ld_new;
	icp.params[:firstGuess] = u
#	icp.params[:firstGuess] = GSL::Vector.alloc(-0.2,0.1,deg2rad(4)).col

	icp.scan_matching

	ld_ref = ld_new;
	end
end

scan_matching