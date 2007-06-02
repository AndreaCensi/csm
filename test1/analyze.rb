#!/usr/bin/env ruby

require 'rubygems'
require 'json/pure'
require 'gsl'

include GSL

def read_samples(string)
	samples = []
	j = JSON::Pure::Parser.new(string)
	while true
		j.scan(/\s/) 
		break if j.eos?
		jo = j.parse
		$stderr.write '.'
		true_e = Vector.alloc jo['true_e']
		samples.push true_e.trans
	end	
	$stderr.puts "Read #{samples.size}"
	samples
end

class GSL::Vector
	def as_pose
		x_mm = self[0] * 1000
		y_mm = self[1] * 1000
		th_deg = rad2deg(self[2])
		"(#{x_mm}mm, #{y_mm}mm,  #{th_deg}deg)"
	end
end

def rad2deg(x);  x * 180 / Math::PI  end
def cov(samples, mean = nil)
	cov = (samples[0]-samples[0])
	cov = cov.trans * cov
end

def mean(samples)
	mean = samples[0]-samples[0]
	samples.each {|x| mean += x}
	mean / samples.size
end

samples = read_samples($stdin.read)

p mean(samples).as_pose





