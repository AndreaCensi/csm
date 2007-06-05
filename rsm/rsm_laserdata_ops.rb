require 'rsm_laserdata'


class LaserData
	
	def add_noise!(sigma, rng)
		readings.map! { |x| 
			x + rng.gaussian(sigma)
		}
	end
	
	def deep_copy
		Marshal.load(Marshal.dump(self))
	end
	
	def compute_cartesian
		for i in 0..nrays-1
			next if not valid? i
			@p[i] = v(i) * readings[i]
		end
	end
end
