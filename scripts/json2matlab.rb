#!/usr/bin/env ruby
for s in ['../rsm', '.']
	$LOAD_PATH << File.expand_path(File.join(File.dirname(__FILE__), s))
end

require 'rubygems'
require 'json/pure'

require 'fix_json'
require 'json_utils'

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
		all? {|x| x.kind_of?(Numeric) or x.nil? }
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
				v = self[k]
				if v.kind_of?(Array) and (not v.all_numbers?) and (not v.is_matrix?)
				"'#{k}', {#{v.to_matlab}}"
				elsif v.kind_of?(Hash)
				"'#{k}', {#{v.to_matlab}}"
				else
				"'#{k}', #{v.to_matlab}"
				end
			}.join(", ... \n\t") +
		")"
	end
end

class Object
	def to_matlab(); to_s; end
end

class String
	def to_matlab(); "'" + to_s + "'"; end
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

class NilClass
	def to_matlab
		"NaN"
	end
end

def main_json2matlab
	if ARGV.any? {|x| x =~ /help/}
		$stderr.puts "Three usages:"
		$stderr.puts " 1) without parameters, it acts as a filter; out is a Matlab structure "
		$stderr.puts "       $ json2matlab.rb  < in.json > out.m "
		$stderr.puts " 2) with one parameter, it creates <in>.m (a function named <in>) "
		$stderr.puts "       $ json2matlab.rb in.json "
		$stderr.puts " 3) with two parameters, it creates `f.m` (a function named 'f') "
		$stderr.puts "       $ json2matlab.rb in.json f "
		$stderr.puts "       $ json2matlab.rb - f         reading from stdin"
		exit 0
	end


	io =  if ( (file = ARGV[0]) && (file != '-') ) then File.open(file) else $stdin end
	complete_file = io.read

	a = read_all_objects(complete_file)
	$stderr.write "json2matlab: Found #{a.size} JSON objects. "
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
		if ARGV[1]
			basename = ARGV[1]
		end
	
		output_file = File.join(File.dirname(file), basename + ".m")
		$stderr.puts "Writing to #{output_file.inspect}."
		File.open(output_file, 'w') do |f|
			f.puts "function res = #{basename}"
			f.puts "res = ..."
			f.puts(a.to_matlab + ";")
		end
	else
		$stdout.puts(a.to_matlab + ";")	
	end

end


main_json2matlab if File.basename($0) == 'json2matlab.rb'

