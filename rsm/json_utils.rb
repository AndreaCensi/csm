require 'fix_json'

def read_all_objects(string, progress_string=nil)
	a = []
	j = JSON::Pure::Parser.new(string)
	while true
		j.scan(/\s*/) 
		break if j.eos?
		begin
			a.push j.parse
			$stderr.write progress_string if progress_string
		rescue Exception => e
			$stderr.puts "After #{a.size} objects: #{e}" 
			return a
		end
	end	
	a
end
