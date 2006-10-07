require 'rsm_laserdata'

module GSL
	class Vector
		def marshal_dump
			to_a
		end
		
		def marshal_load(a)
	#		puts "A is a #{a.class}: #{a}"
			a.each_index { |i| 
	#			puts "i=#{i.class} a(i) = #{a[i].class}"
			}
		end
	end
end

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
			p[i] = v(i) * readings[i]
		end
	end
end
