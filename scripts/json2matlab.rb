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

class Object 
	def recurse_json(&block) end
end
class Hash
	def recurse_json(&block) 
		each do |k, v|
			self[k]=block.call(v, self)
			self[k].recurse_json(&block)
		end
	end
end

class Array
	def recurse_json(&block) 
		each_index do |i|
			self[i] = block.call(self[i], self)
			self[i].recurse_json(&block)
		end
	end
end

io =  if file = ARGV[0] then File.open(file) else stdin end
	
a = read_all_objects(io.read)
$stderr.puts "Found #{a.size} JSON objects."
a = a[0] if a.size == 1 

a.recurse_json do |child, parent| 
	if parent.kind_of?(Array) and child.nil? then
		(0.0/0.0) 
	else
		 child
	end
end

if file = ARGV[0]
	basename = File.basename(file).gsub(/\.\w*$/,'')
	output_file = File.join(File.dirname(file), basename + ".m")
	$stderr.puts "Writing to #{output_file}"
	File.open(output_file, 'w') do |f|
		f.puts "function res = #{basename}"
		f.puts "res = ..."
		f.puts(a.to_matlab + ";")
	end
else
	$stdout.puts(a.to_matlab + ";")	
end

