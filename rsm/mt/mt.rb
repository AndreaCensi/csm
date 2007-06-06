#!/usr/bin/env ruby

require 'rsm_sm_loop'


class Panorama < LaserData
	SIGMA = 0.005
	COMP_THRESHOLD = SIGMA * 5
	SCALE = 1
	
	attr_accessor :parent
	
	def angle2ray(phi)
		i = (phi-@min_theta) / (@max_theta-@min_theta) * @nrays
		i = i.round 
		i < 0 || i >= @nrays ? nil : i
	end
	
	def initialize(estimate, odometry)
		super(721)
		
		@min_theta = -Math::PI
		@max_theta = +Math::PI
		for i in 0..@nrays-1
			@theta[i] = @min_theta + (@max_theta-@min_theta) *
			 	i / (@nrays-1);
		end
		@estimate = estimate.clone
		@odometry = odometry.clone
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
					
					dist = sin( (phi-@theta[j]).abs ) * rho
					
					cov_rho = (SIGMA**2) + (dist**2) / SCALE
					cov_old_rho = @cov_readings[j]
					cov_new_rho = 1 /( 1 / cov_old_rho + 1 / cov_rho)
					new_rho = cov_new_rho * (old_rho / cov_old_rho + rho / cov_rho);

					@readings[j] = new_rho
					@cov_readings[j] = cov_new_rho
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



def quick_match(scan_matcher_klass, params, ref, sens, guess)
	sm = scan_matcher_klass.new
	sm.params = params
	sm.params = params 
	sm.params[:laser_ref] = ref;
	sm.params[:laser_sens] = sens;
	sm.params[:firstGuess] = guess
	res = sm.scan_matching
	res
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
	panorama = Panorama.new(first.estimate, first.odometry)
	panorama.merge(first)
	laser_ref = first

	results = []
	count = 0
	
	until input.eof?
		laser_sens = LogReader.shift_laser(input)
		
		res = quick_match(scan_matcher_klass, params, laser_ref, laser_sens, 
			pose_diff(laser_sens.odometry, laser_ref.odometry))
		break if not res[:valid]
		error = res[:error]
		
		puts "mt: match laser_ref #{res[:nvalid]} #{res[:avg_error] }   #{pv(res[:x])} "

		laser_sens.estimate = oplus(laser_ref.estimate, res[:x])
	
		n = panorama.merge(laser_sens)

		if n < 200
			if parent = panorama.parent
				guess = pose_diff(panorama.estimate, parent.estimate);
				res = quick_match(scan_matcher_klass, params, parent, panorama, 
					guess)
				if res[:valid]
					delta = pose_diff(res[:x], guess)
					puts "mt: Match with parent: #{res[:avg_error]} #{res[:nvalid]} #{pv(delta)}"
					if (res[:avg_error] < 1) && (res[:nvalid] > 150)
						puts "mt: parent-correct!"
						panorama.estimate = oplus(panorama.estimate, delta)
						laser_sens.estimate = oplus(laser_sens.estimate, delta)
					end
				end
			end
			
			output.puts panorama.to_json
			new_panorama = Panorama.new(laser_sens.estimate, laser_sens.odometry)
			new_panorama.parent = panorama
			new_panorama.merge(laser_sens)
			panorama = new_panorama
			count = 0
			puts "mt: new panorama ##{count}: error #{error}"
		else
			puts "mt: continuing ##{count}: error #{error}"
		end
		output_scans.puts laser_sens.to_json
		
		puts "mt: merged #{n}"

		laser_ref = laser_sens
		count += 1
	end
	
end


go_mt
	

