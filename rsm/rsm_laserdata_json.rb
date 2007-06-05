#require 'rubygems'
require 'json/pure'

class Array
	def nan_to_nil!
		each_index do |i|
			v = self[i]
			self[i] = nil if v.kind_of? Float and v.nan?
			self[i] = self[i].to_f  if self[i]
		end
		self
	end
	
	def all_nan?
		all? {|v| v.kind_of? Float and v.nan?}
	end
end

class LaserData
	
	def to_json(*a)
		h = {
			'nrays'       => @nrays,
			'min_theta'   => @min_theta,
			'max_theta'   => @max_theta,

			'valid'       => @valid.map{|x| x ? 1 : 0},
			'readings'    => @readings.clone.nan_to_nil!,
			'theta'       => @theta   .clone.nan_to_nil!,

			'odometry'    => @odometry .to_a.nan_to_nil!,
			'estimate'    => @estimate .to_a.nan_to_nil!,
			'true_pose'   => @true_pose.to_a.nan_to_nil!,
		}
		h['cluster']     = @cluster unless @cluster.all?{|x| x == -1}
		h['alpha_valid'] = @alpha_valid unless @alpha_valid.all?{|x| not x}
		h['alpha'] = @alpha.clone.nan_to_nil! unless @alpha.all_nan?
		h['cov_alpha'] = @cov_alpha.clone.nan_to_nil! unless @cov_alpha.all_nan?
		h['cov_readings'] = @cov_readings.clone.nan_to_nil! unless @cov_readings.all_nan?
		
#		corrd = corr.map{|x| x.valid ? [x.j1, x.j2] : nil }
#		h['corresponde] = c
		h.to_json(*a)
	end

end


if File.basename($0) == 'rsm_laserdata_json.rb' 
	# testing
	ld = LaserData.new(10)
	ld.readings[8] = 42
	# ld.corr[i].valid = true
	# ld.corr[i].j1 = 12
	# ld.corr[i].j2 = 13
	
	require 'sm'
	include Sm
	json = ld.to_json
	puts "\nLaserData.to_json:\n"
	p json
	c_jo = json_parse(json)
	puts "\nLaserData.to_json -> c_jo = json_parse -> json_write(c_jo) :\n"
	p json_write(c_jo)
	c_ld = json_to_ld(c_jo)
	puts "\nLaserData.to_json -> json_parse -> json_to_ld -> ld_to_json: \n"
	c_jo2 = ld_to_json(c_ld)
	p json_write(c_jo2)
	
end