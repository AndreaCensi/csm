
class LaserData
	# Parses a CARMEN line
	def LaserData.convert_line(l)
		tokens = l.split
		tokens.shift
		nrays = tokens.shift.to_i

		if nrays <= 0
			raise "Bad no. of rays (#{ld.nrays}) in '#{l}'"  end
		if tokens.size < nrays+9 
			then raise "Line incomplete: '#{l}'" end
		
		ld = LaserData.new(nrays)
				
		ld.min_reading   = 0.001;
		ld.max_reading   = 49;
		ld.min_theta     = -PI/2;
		ld.max_theta     =  PI/2;

		for i in 0..nrays-1
			if tokens.empty? 
				raise "Could not read ray#{a} in '#{l}'" end
	
			reading = tokens.shift.to_f
			
			
			if (not reading) || (reading<0)
				raise "Bad value (#{p.reading}) for ray#{a} in '#{l}'"
			end
			
			ld.theta[i] = ld.min_theta + i * (ld.max_theta) / (ld.nrays-1)
			ld.valid[i] = (reading < ld.max_reading) && (reading > ld.min_reading)
			ld.readings[i] = ld.valid[i] ? reading : GSL::NAN

		end
		
		ld.estimate      = parse_tokens(tokens);
		ld.odometry      = parse_tokens(tokens);
		ld.ipc_timestamp = tokens.shift
		ld.timestamp     = tokens.shift
		ld.hostname      = tokens.shift
		ld
	end
	
	def to_carmen
		line = "FLASER #{nrays} " + readings.join(' ') +
			"\t #{estimate[0]} #{estimate[1]} #{estimate[2]} "+
			"\t #{odometry[0]} #{odometry[1]} #{odometry[2]} "+
			"\t #{ipc_timestamp} #{timestamp} #{hostname}"
	end
	
end

def parse_tokens(tokens)
	v = GSL::Vector.alloc(GSL::NAN,GSL::NAN,GSL::NAN).col
	v[0] = tokens.shift.to_f
	v[1] = tokens.shift.to_f
	v[2] = tokens.shift.to_f
	v
end






