#!/usr/bin/env ruby

require 'rsm_sm_loop'

def get_empty_panorama(estimate)
	ld = LaserData.new(721)
	ld.min_theta = -Math::PI
	ld.max_theta = +Math::PI
	for i in 0..ld.nrays-1
		ld.theta[i] = ld.min_theta + (ld.max_theta-ld.min_theta) *
		 	i / (ld.nrays-1);
	end
	ld.estimate = estimate.clone
	ld.odometry = estimate.clone
	ld.cov_readings = [GSL::NAN]*ld.nrays
	ld
end

class LaserData
	SIGMA = 0.005
	COMP_THRESHOLD = SIGMA * 5
	
	attr_accessor :cov_readings
	
	def angle2ray(phi)
		i = (phi-@min_theta) / (@max_theta-@min_theta) * @nrays
		i = i.round 
		i < 0 || i >= @nrays ? nil : i
	end
	
	def merge(ld)
		count = 0
		ld.compute_cartesian
		# pose of second scan in first scan's frame
		diff = pose_diff(ld.estimate, self.estimate)
		for i in 0 ... ld.nrays-1
 			if not ld.valid? i
#				$stderr.puts "Skipping #{i}"
				next
			end
			# coordinates of point i of second scan 
			# relative to first scan frame
			p = transform(ld.p[i], diff)
			
			phi = atan2(p[1], p[0])
			rho = p.nrm2
			
			if j = angle2ray(phi)
				old_rho = @readings[j]
				new_rho = nil
				
				if old_rho.nan? 
					@readings[j] = rho
					@cov_readings[j] = SIGMA**2;
					@valid[j] = true
#					$stderr.puts "#{i}: Init #{j}"
					count += 1
					next
				end
				
				compatible = (rho-old_rho).abs < COMP_THRESHOLD
				if compatible
					old_cov = @cov_readings[j]
					new_cov = 1 /( 1 / old_cov + 1 / (SIGMA**2))
					new_rho = new_cov * (old_rho / old_cov + rho / (SIGMA**2));

					@readings[j] = new_rho
					@cov_readings[j] = new_cov
					count += 1
				end
#				$stderr.puts "#{i} #{j}, #{old_rho} => #{new_rho}"
			else
#				$stderr.puts "#{i} no index for phi = #{phi}"
			end
		end
		count
	end
	
end


def go_mt
	if ARGV.size < 3
		puts "sm <SCANMATCHER> '[]' <input> <output> "
		exit 1
	end

	scan_matcher_klass = eval ARGV[0]
	scan_list = eval ARGV[1]

	#puts "Scan_list = #{scan_list}"

	input = File.open ARGV[2]
	output = File.open(ARGV[3], "w")
	output_scans = File.open(File.basename(ARGV[3]) + "_scans.log", "w")
	
	params = standard_parameters
	
	include MathUtils

	first = LogReader.shift_laser(input)
	first.estimate = first.odometry
	panorama = get_empty_panorama(first.estimate)
	panorama.merge(first)
	laser_ref = first

	results = []
	count = 0
	
	until input.eof?

		laser_sens = LogReader.shift_laser(input)
		min_step = -0
		min_theta_step_deg = -5;

		u =  pose_diff(laser_sens.odometry, panorama.odometry)

		if (u[0,1]).nrm2 <= min_step && (u[2].abs <deg2rad(min_theta_step_deg))

			# todo: merge readings
			#puts "Skipping (same odometry)"
			next
		end

#		puts "Ref: #{pv(laser_ref.odometry)}"
#		puts "New: #{pv(laser_sens.odometry)}"

		if (not scan_list.empty?) && (not scan_list.include? count)
			break if count > scan_list.max
			count+=1
			laser_ref = laser_sens;
			next
		end

		sm = scan_matcher_klass.new
		# Write log of the icp operation

		if scan_list.include? count
			sm.journal_open("rsm_sm.#{sm.name}.#{count}.txt")
		end

		sm.params = params 
		sm.params[:laser_ref] = laser_ref;
		sm.params[:laser_sens] = laser_sens;
		sm.params[:firstGuess] = pose_diff(laser_sens.odometry, laser_ref.odometry)
		res = sm.scan_matching
		break if not res[:valid]
		x = res[:x]
		error = res[:error]
		iterations = res[:iterations]

		# sm.params = params 
		# sm.params[:laser_ref] = panorama;
		# sm.params[:laser_sens] = laser_sens;
		# sm.params[:firstGuess] = pose_diff(laser_sens.odometry, panorama.odometry)
		# res_pan = sm.scan_matching
		# res_pan[:error] = res_pan[:error] / res_pan[:nvalid]
		# res[:error] = res[:error] / res[:nvalid]
		# break if not res_pan[:valid]

		puts "mt: match laser_ref #{res[:error] }   #{pv(res[:x])} #{res[:nvalid]}"
#		puts "mt: match       pan #{res_pan[:error]}   #{pv(res_pan[:x])}"

		# use_pan = res_pan[:error] < res[:error]
		# use_pan = false
		# if use_pan
		# 	puts "mt: Using pan!"
		# 	laser_sens.estimate = oplus(panorama.estimate, res_pan[:x])
		# else
			laser_sens.estimate = oplus(laser_ref.estimate, res[:x])
#		end
	
		n = panorama.merge (laser_sens)

#		if Vector[res_pan[:x][0],res_pan[:x][1]].nrm2 > 0.5
		if n < 200
			output.puts panorama.to_json
			panorama = get_empty_panorama(laser_sens.estimate)
			panorama.odometry = laser_sens.odometry.clone
			panorama.merge(laser_sens)
			count = 0
			puts "mt: new panorama ##{count}: error #{error}"
		else
			puts "mt: continuing ##{count}: error #{error}"
		end
		output_scans.puts laser_sens.to_json
		
		puts "mt: merged #{n}"

#		q = laser_sens.estimate
		#$stderr.puts "rsm_sm.rb: #{count} time=#{realtime} error = #{error} it = #{iterations} x = #{pv(x)} u = #{pv(u)} q = #{pv(q)}"

		laser_ref = laser_sens
		count += 1
	end
	
end


go_mt
	

