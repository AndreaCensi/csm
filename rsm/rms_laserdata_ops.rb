require 'rms_laserdata'


class LaserData
	
	def add_noise!(sigma, rng)
		for p in points
			noise = rng.gaussian(sigma)
			p.reading += noise
		end
	end
	
    def deep_copy
      Marshal.load(Marshal.dump(self))
    end
end
