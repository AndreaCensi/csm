begin
require 'rubygems'
rescue => ex
	$stderr.puts  "Rubygems not available"
end

require 'json/pure'

class Array
	def nan_to_nil!
		for i in 0..size-1
			v = self[i]
			self[i] = nil if v.kind_of? Float and v.nan?
			self[i] = self[i].to_f  if self[i]
		end
		self
	end
	
	def nil_to_nan!
		for i in 0..size-1
			self[i] = self[i] || GSL::NAN
		end
		self
	end
	
	def all_nan?
		all? {|v| v.kind_of? Float and v.nan?}
	end
end

class GSL::Vector
	def to_json(*a)
		to_a.nan_to_nil!.to_json(*a)
	end
end

class LaserData
	
	def to_json(*a)
#		puts @odometry.to_s
		h = {
			'nrays'       => @nrays,
			'min_theta'   => @min_theta,
			'max_theta'   => @max_theta,

			'valid'       => @valid.map{|x| x ? 1 : 0},
			'readings'    => @readings.clone.nan_to_nil!,
			'theta'       => @theta   .clone.nan_to_nil!,

			'odometry'    => @odometry,
			'estimate'    => @estimate,
			'true_pose'   => @true_pose
		}
		
		h['cluster']     = @cluster unless @cluster.all?{|x| x == -1}
		h['alpha_valid'] = @alpha_valid.map{|x| x ? 1 : 0} unless @alpha_valid.all?{|x| not x}
		h['alpha'] = @alpha.clone.nan_to_nil! unless @alpha.all_nan?
		h['cov_alpha'] = @cov_alpha.clone.nan_to_nil! unless @cov_alpha.all_nan?
		h['cov_readings'] = @cov_readings.clone.nan_to_nil! unless @cov_readings.all_nan?
		
#		corrd = corr.map{|x| x.valid ? [x.j1, x.j2] : nil }
#		h['corresponde] = c
		h.to_json(*a)
	end

	def from_hash(h)
		self.min_theta = h['min_theta'].to_f
		self.max_theta = h['max_theta'].to_f
		self.valid     = h['valid'].map{ |x| (x == 0 || !x ) ? false : true } 
		self.readings  = h['readings'].clone.nil_to_nan!
		self.theta     = h['theta'].clone.nil_to_nan!
		
		self.cluster = h['cluster'].clone if h['cluster']
		self.alpha_valid = h['alpha_valid'].clone if h['alpha_valid']
		self.alpha = h['alpha'].clone.nil_to_nan! if h['alpha']
		self.cov_alpha = h['cov_alpha'].clone.nil_to_nan! if h['cov_alpha']
		self.cov_readings = h['cov_readings'].clone.nil_to_nan! if h['cov_readings']	
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