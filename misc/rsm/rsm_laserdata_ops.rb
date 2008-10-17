require 'rsm_laserdata'


class LaserData
	
	def add_noise!(sigma, rng)
		readings.map! { |x| 
			x + rng.gaussian(sigma)
		}
	end
	
	def LaserData.from_json(json_string)
		h = JSON.parse(json_string)
		ld = LaserData.new(h['nrays'])
		ld.from_hash(h)
		ld
	end
	
	def deep_copy
		LaserData.from_json(self.to_json)
	end
	
	def compute_cartesian
		for i in 0..nrays-1
			next if not valid? i
			@p[i] = v(i) * readings[i]
		end
	end
end
