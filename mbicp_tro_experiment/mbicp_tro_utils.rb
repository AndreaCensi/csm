
require 'laserdata'
require 'gsl'
require 'icp'
require 'icpc_wrap'

include MathUtils
include Math

# Fill other fields with standard values
def create_scan(rays, odometry)
	ld = LaserData.new
	ld.nrays = rays.size
	ld.min_reading   = 0.1;
	ld.max_reading   = 49;
	ld.min_theta     = -PI/2;
	ld.max_theta     =  PI/2;
	ld.points = Array.new
	rays.each_index do |i|
		p = LaserPoint.new
		p.theta = -PI/2 + i*PI/ld.nrays
		p.intensity = 0; # NAN
		p.reading = rays[i]<8 ? rays[i] : 0
		p.valid = true
		ld.points.push p
	end
	ld
end

def read_log(io)
	scans = Array.new
	nscan, nrays = io.gets.split.map{|s| s.to_i}
	puts "nscan = #{nscan}, nrays = #{nrays}"
	begin
	nscan.times do 
		rays = Array.new
		nrays.times do rays.push io.gets.to_f end
		odometry = io.gets.split.map{|s| s.to_f}
		scans.push create_scan(rays, odometry);
	end
	rescue
		puts "File truncated at line #{io.lineno}, "+
		     " already read #{scans.size} scans; expected #{nscan}."
	end
	scans
end

def random_displacement(max_disp) 
	disp = max_disp.clone;
	disp[0] = disp[0] * 2.0*(rand-0.5)
	disp[1] = disp[1] * 2.0*(rand-0.5)
	disp[2] = disp[2] * 2.0*(rand-0.5)
	disp
end



def two_decimals(x)
	(x*100).round/100.0;
end

def pad(s, n)
	if s.length < n
		return s + " "*(n-s.length)
	end
	return s
end

def biggest_non_empty_bin(hist)
	b = hist.bins - 1;
	while (hist.get(b)==0) and (b>0)
		b-=1
	end
	b
end

def write_summary(io, hist)
	sum = hist.sum;
	for b in (0..biggest_non_empty_bin(hist))
		range = hist.get_range(b)
		io.write pad("#{range[0]} <= x < #{range[1]}",20)
		io.write "\t\t"
		io.write "#{two_decimals(hist.get(b)*100.0/sum)}%\n"
	end
end

