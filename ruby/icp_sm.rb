#!/usr/bin/env ruby
require 'logreader'

require 'icp'

def scan_matching(io)
	include MathUtils
	
	log = LogReader.read_log(io)

	def next_ld(log)
#	do
#		e = log.shift
#		if e.kind_of? LaserData then e
#	until e.kind_of?(LaserData)
		log.shift
	end

	puts "Start processing: #{log.size}"

	ld_ref = next_ld(log)
	
	count = 0
	until log.empty?
		ld_new = next_ld(log)
		min_step = 0.2

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
		icp.journal_open("icp_sm.rb-sm#{count+=1}.txt")

		icp.params[:maxAngularCorrectionDeg]= 30
		icp.params[:maxLinearCorrection]=  0.2
		icp.params[:laser_ref] = ld_ref;
		icp.params[:laser_sens] = ld_new;
		icp.params[:firstGuess] = u

#		icp.params[:laser_sens] = ld_ref;
#		icp.params[:firstGuess] = GSL::Vector.alloc(0.2,0.2,0)
	
		icp.scan_matching

		ld_ref = ld_new;
	end
end

io = ARGV.size>0 ? File.open(ARGV[0]) : $stdin 

scan_matching(io)

