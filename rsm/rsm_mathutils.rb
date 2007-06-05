require 'gsl'

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
		
		def any_nan?
			for i in 0..size-1
				return true if self[i].nan?
			end
			false
		end
		
	end
end

module MathUtils 
	public
	def deg2rad(d)
		 d * Math::PI / 180
	end
	
	def rad2deg(d)
		 d / Math::PI * 180
	end
	
	## versor
	def vers(a) 
		GSL::Vector.alloc(Math.cos(a), Math.sin(a)).col
	end

	def two_decimals(x)
		x.nan? ? GSL::NAN : (x*100).round/100.0;
	end
	
	def pv(x)
		"[#{two_decimals(x[0]*1000) }mm,#{two_decimals(x[1]*1000)}mm,"+
		"#{two_decimals(rad2deg(x[2]))}deg]"
	end

	def pm(m)
		std_x = (sqrt(m[0,0])*100000).round/100.0 # in mm
		std_y = (sqrt(m[1,1])*100000).round/100.0 # in mm
		std_th = (rad2deg(sqrt(m[2,2]))*100).round/100.0
		"[+-#{2*std_x}mm,+-#{2*std_y}mm,+-#{2*std_th}]deg"
	end
	## 2x2 rotation matrix
	def rot(a)
		GSL::Matrix[[Math.cos(a), -Math.sin(a)], [Math.sin(a), Math.cos(a)]]
	end
	
	def transform(point, x)
		t = x[0,1]; theta = x[2];
		rot(theta)*point + t
	end

	
	def pose_diff(to,from)
		oplus(ominus(from), to)
	end
	
	def oplus(x1,x2)
		theta = x1[2];
		GSL::Vector.alloc(
			x1[0] + x2[0]*Math.cos(theta) - x2[1]*Math.sin(theta),
			x1[1] + x2[0]*Math.sin(theta) + x2[1]*Math.cos(theta),
			x1[2] + x2[2]).col
	end
	
	def ominus(x)
		th = x[2]
		GSL::Vector.alloc(
			-x[0]*Math.cos(th) - x[1]*Math.sin(th),
			 x[0]*Math.sin(th) - x[1]*Math.cos(th),
			-x[2]).col
	end
	
	def J1(x1,x2)
		Matrix[
			[1, 0, -x2[0]*sin(x1[2])-x2[1]*cos(x1[2])],
			[0, 1,  x2[0]*cos(x1[2])-x2[1]*sin(x1[2])],
			[0, 0, 1]
		]
	end
	
	def J2(x1,x2)
		Matrix[
			[cos(x1[2]), -sin(x1[2]), 0],
			[sin(x1[2]),  cos(x1[2]), 0],
			[0, 0, 1]
		]
	end
	
	
	def angleDiff(a,b)
		d = a - b
		
		while d < -PI; d+=2*PI; end
		while d >  PI; d-=2*PI; end
		d
	end
	
	# from, to, start_cell, range = 
	def possible_interval(point, ld, max_angular_correction_deg, 
		max_linear_correction)
		
		angleRes =  (ld.max_theta-ld.min_theta)/ld.nrays;

		# Delta for the angle
		delta = deg2rad(max_angular_correction_deg).abs +
		        Math.atan(max_linear_correction/point.nrm2).abs;

		# Dimension of the cell range
		range = (delta/angleRes).ceil;
		
		# To be turned into an interval of cells
		start_theta = Math.atan2(point[1],point[0]);
		
		start_cell  = 
			(start_theta - ld.min_theta) / (ld.max_theta-ld.min_theta) * ld.nrays;
	
		start_cell = start_cell.ceil
		
		# Pay attention to minimum and maximum
		from = min(ld.nrays-1, max( (start_cell-range).floor,0));
		to = max(0, min((start_cell+range).ceil,ld.nrays-1));
	
		if false
			puts "start_theta=#{rad2deg(start_theta)}°, "+
			     "range = #{range} cells,"+
			     "start_cell=#{rad2deg(start_cell)},"+
				  "fromto=#{from}->#{to}"
		end
		
		return from, to, start_cell, range
	end
	
	
end