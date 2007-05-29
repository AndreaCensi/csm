#!/usr/bin/env ruby
require 'rubygems'
require 'json/pure'

def read_all_objects(string)
	a = []
	j = JSON::Pure::Parser.new(string)
	while true
		j.scan(/\s/) 
		break if j.eos?
		a.push j.parse
	end	
	a
end

class Array
	def matrix_size?(x) [9].include?x end
	def to_matlab
		if is_matrix?
			write_as_matrix
		elsif all_numbers?
			"[  "+map{|x| x.to_matlab }.join("; ")+"]"
		else
			"{ ... \n "+map{|x| x.to_matlab }.join(",  ... \n")+"}"
		end
	end
	
	def all_numbers?
		all? {|x| x.kind_of? Numeric}
	end
	
	def is_matrix?
		return false if empty? or not all? {|x| x.kind_of?(Array) && x.all_numbers?}
		columns = first.size
		return false if not all? {|x| x.size==columns}
		true
	end
	
	def write_as_matrix
		"[ " + map{|row| row.join(", ")}.join("; ... \n") +  "]"
	end
end

class Hash
	def to_matlab
		" struct(" +
			keys.map{ |k|
				if self[k].kind_of? Hash
				"'#{k}', {#{self[k].to_matlab}}"
				else
				"'#{k}', #{self[k].to_matlab}"
				end
			}.join(", ... \n\t") +
		")"
	end
end

class Object
	def to_matlab() to_s end
end
	
	
a = read_all_objects($stdin.read)
$stderr.puts "found #{a.size} objects"
a = a[0] if a.size == 1 

if function = ARGV[0]
	puts "function res = #{function}"
	puts "res = ..."
	puts(a.to_matlab + ";")
else
	puts(a.to_matlab + ";")	
end

