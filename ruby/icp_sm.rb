#!/usr/bin/env ruby
require 'logreader'
require 'icp'

include Pose

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
	min_step = 0.1
	if (ld_new.odometry[0,1]-ld_ref.odometry[0,1]).nrm2 <= min_step 
		# todo: merge readings
		puts "Skipping (same odometry)"
		next
	end

	icp = ICP.new

	icp.params[:maxAngularCorrectionDeg]= 10
	icp.params[:maxLinearCorrection]=  0.4

	icp.params[:laser_ref] = ld_ref;
	icp.params[:laser_sens] = ld_new;
#	icp.params[:firstGuess] = Pose.minus(ld_new.odometry, ld_ref.odometry)
	
	icp.scan_matching
	
	ld_ref = ld_new;
end
