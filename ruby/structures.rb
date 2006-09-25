#!/usr/bin/env ruby
require("gsl")

class Event
	attr_accessor :timestamp
	attr_accessor :ipc_timestamp
	attr_accessor :hostname
	
	
end

module MathUtils
	include Math
	public
	
	## versor
	def vers(a); GSL::Vector.alloc(cos(a), sin(a)); end

	## 2x2 rotation matrix
	def rot(a)
		GSL::Matrix[[cos(a), -sin(a)], [sin(a), cos(a)]]
	end
	
	def transform(point, x)
		t = x[0,1]; theta = x[2];
		rot(theta)*point + t
	end
end

module Pose 

	def Pose.parse_tokens(tokens)
		v = GSL::Vector.alloc(GSL::NAN,GSL::NAN,GSL::NAN)
		v[0] = tokens.shift.to_f
		v[1] = tokens.shift.to_f
		v[2] = tokens.shift.to_f
		v
	end
	
	def minus(x2,x1)
	
	end
	
	def oplus(x1,x2)
	
	end
	
	def ominus(x)
	
	end
	
	def J1(x1,x2)
	
	end
	
	def J2(x1,x2)
	
	end
end

class LaserPoint 
	include MathUtils
	
	attr_accessor :reading
	attr_accessor :intensity
	attr_accessor :theta
	attr_accessor :valid
	
	def valid?
		@valid
	end
	
	def cartesian; vers(theta)*reading; end
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

