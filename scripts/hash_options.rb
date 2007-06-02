
class Hash
	Comment = /^\s*\#/ 
	Option  = /^\s*(\w+)\s*=?\s*(.+)$/ 

	def read_conf(io, dir)
		io.each do |line|
			next if (line.strip.size == 0) || line =~ Comment
			if m = Option.match(line)
				name = m[1]
				value = m[2].strip
				
				begin
					value = eval(value)
				rescue
					
				end
				self[name] = value
#				$stderr.puts  "#{name} = #{value}"
			elsif m = /^\s*<\s*(.*)$/.match(line)
				filename = m[1]
				filename = File.expand_path(File.join(dir, filename))
				read_conf(File.open(filename), File.dirname(filename))
			else
				$stderr.puts "Line #{line.inspect} is malformed"
				exit -1
			end
		end
	end
	
	def config_dump
		s = ""
		max_size = keys.map{|x|x.to_s.size}.max + 2
		for k in keys.sort
			s += "#{k} " + " " * (max_size - k.to_s.size) + self[k].inspect + "\n"
		end
		s
	end
end

if File.basename($0) == 'hash_options.rb'
	h = Hash.new
	h.read_conf $stdin
	p h
end