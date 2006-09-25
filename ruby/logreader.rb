#!/usr/bin/env ruby

require("structures")

class LogReader
	QUIET = true
	ShowProgress = true
	
	def LogReader.read_log(io)
		events = Array.new 
		io.each_line do |l|
			e = convert_line(l)
			events.push(e) if not e.nil?
			if ShowProgress then $stderr.write '.' end
		end
		events
	end
	
	def LogReader.convert_line(l)
		type = l.split[0]
		
		case type
			when 'FLASER' 
				$stderr.write 'l'
				ld = LaserData.convert_line(l);
				return ld
			else
				unless QUIET then $stderr.puts "Type '#{type}' not found." end
				return nil
		end
	end
	
end

#LogReader.read_log($stdin)
