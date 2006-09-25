#!/usr/bin/env ruby
require("gsl")

class Event
	attr_accessor :timestamp
	attr_accessor :ipc_timestamp
	attr_accessor :hostname
	
	
end

class Pose 
#	def initialize
		#alloc(GSL::NAN,GSL::NAN,GSL::NAN)
#	end
	
	def Pose.parse_tokens(tokens)
		v = GSL::Vector.alloc(GSL::NAN,GSL::NAN,GSL::NAN)
		v[0] = tokens.shift.to_f
		v[1] = tokens.shift.to_f
		v[2] = tokens.shift.to_f
		v
	end
	
	def oplus(v1,v2)
	
	end
	
	def ominus(v)
	
	end
	
	def J1(x1,x2)
	
	end
	
	def J2(x1,x2)
	
	end
end

class LaserPoint
	attr_accessor :reading
	attr_accessor :intensity
	attr_accessor :theta
end

class LaserData < Event
	include Math
	
	attr_accessor :odometry
	attr_accessor :estimate
	attr_accessor :points
	attr_accessor :nrays
	attr_accessor :min_reading
	attr_accessor :max_reading
	attr_accessor :min_theta
	attr_accessor :max_theta
	
	public
	def LaserData.convert_line(l)
		ld = LaserData.new
		tokens = l.split
		tokens.shift
		ld.nrays = tokens.shift.to_i
		
		if ld.nrays <= 0 then raise "Bad no. of rays (#{ld.nrays}) in '#{l}'"  end
		if tokens.size < ld.nrays+9 then raise "Line incomplete: '#{l}'" end
		
		ld.points = Array.new
		ld.nrays.times do |a|
			if tokens.empty? then raise "Could not read ray#{a} in '#{l}'" end
	
			p = LaserPoint.new
			p.theta = -PI/2 + a*PI/ld.nrays
			p.intensity = 0; # NAN
			p.reading = tokens.shift.to_f
			
			if (not p.reading) || (p.reading<0)
				raise "Bad value (#{p.reading}) for ray#{a} in '#{l}'"
			end
			
			ld.points.push p
		end
		
		ld.min_reading   = 0.1;
		ld.max_reading   = 49;
		ld.min_theta     = -PI/2;
		ld.max_theta     =  PI/2;
		ld.estimate      = Pose.parse_tokens(tokens);
		ld.odometry      = Pose.parse_tokens(tokens);
		ld.ipc_timestamp = tokens.shift
		ld.timestamp     = tokens.shift
		ld.hostname      = tokens.shift
		ld
	end
end

