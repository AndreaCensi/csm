require 'rsm'

class LaserData
	
def simple_clustering(threshold)
	count = 0;
	last_reading = nil;
	for i in 0..nrays-1
		if not valid? i
			@cluster[i] = -1
			next
		end

		if (not last_reading.nil?) and
			(last_reading-@readings[i]).abs > threshold
			count += 1
		end

		@cluster[i] = count;
		last_reading = @readings[i]
	end
end

end