def execute_cmd_verb(cmd, verbose, exit_on_error)
	$stderr.puts "$ " + cmd
	
	if verbose
		system cmd
	else
		output = `#{cmd}`
	end
	
	if exit_on_error && $?.exitstatus != 0
		$stderr.puts "\n\n\t command:\n\n\t#{cmd.inspect}\n\n\tFAILED, exit status = #{$?.exitstatus}\n\n"
		exit($?.exitstatus || -1)
	end
	
	$?.exitstatus
end

def execute_cmd(m, exit_on_error=true); execute_cmd_verb(m, true, exit_on_error); end

def search_cmd(program, additional_paths = nil)
	if File.exists? program
    return program
  end
  
	path = ENV['PATH'].split(":") 
	path.push File.dirname($0)
	path.push Dir.pwd
	path = path + [*additional_paths] if additional_paths
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


