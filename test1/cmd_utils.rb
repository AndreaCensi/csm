def execute_cmd(m)
	puts "$ " + m 
	system m
	if $?.exitstatus != 0
		$stderr.puts "\n\n\t command:\n\n\t#{m}\n\n\tFAILED\n\n"
		exit($?.exitstatus)
	end
end

def search_cmd(program, additional_paths = nil)
	path = ENV['PATH'].split(":") 
	path.push File.dirname($0)
	path.push Dir.pwd
	path = path + additional_paths if additional_paths
	for dir in path
		p = File.join(dir, program)
		if File.exists? p
#			$stderr.puts "Using #{p.inspect}"
			return p
		end
	end
	$stderr.puts "Could not find program #{program.inspect}"
	$stderr.puts "Searched in #{path.inspect}"
	exit -1
end

alias find_cmd search_cmd


