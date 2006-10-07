
class LaserData
	# Parses a CARMEN line
	def LaserData.convert_line(l)
		ld = LaserData.new
		tokens = l.split
		tokens.shift
		ld.nrays = tokens.shift.to_i
		
		if ld.nrays <= 0
			raise "Bad no. of rays (#{ld.nrays}) in '#{l}'"  end
		if tokens.size < ld.nrays+9 
			then raise "Line incomplete: '#{l}'" end
		
		ld.min_reading   = 0.1;
		ld.max_reading   = 49;
		ld.min_theta     = -PI/2;
		ld.max_theta     =  PI/2;

		ld.points = Array.new
		ld.nrays.times do |a|
			if tokens.empty? 
				raise "Could not read ray#{a} in '#{l}'" end
	
			p = LaserPoint.new
			p.theta = -PI/2 + a*PI/ld.nrays
			p.intensity = 0; # NAN
			p.reading = tokens.shift.to_f
			
			if (not p.reading) || (p.reading<0)
				raise "Bad value (#{p.reading}) for ray#{a} in '#{l}'"
			end
			p.valid = (p.reading < ld.max_reading) && (p.reading > ld.min_reading)
			
			ld.points.push p
		end
		
		ld.estimate      = Pose.parse_tokens(tokens);
		ld.odometry      = Pose.parse_tokens(tokens);
		ld.ipc_timestamp = tokens.shift
		ld.timestamp     = tokens.shift
		ld.hostname      = tokens.shift
		ld
	end
end