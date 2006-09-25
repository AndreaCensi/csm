#!/usr/bin/env ruby

require 'getoptlong'

def help
<<EOF

	This script parses a Matlab script and finds its dependencies on
	other scripts.

EOF
end

class DB
	# Files required (full path)
	attr_accessor :required
	# Functione whose file was not found
	attr_accessor :notfound
	
	def initialize()
		@name2file = Hash.new
		@required = Array.new
		@notfound = Array.new
	end

	def explore_directory(d)
		pwd = Dir.pwd
		Dir.chdir(d)
		Dir.glob('**/*.m').each do |x| 
			name = File.basename(x,'.m');
			@name2file[name]=File.join(d, x);
#			puts "Found function '#{name}' in file #{x}."
		end
		Dir.chdir(pwd)
	end
	
	def need_function(n)
		if @required.include?(@name2file[n])
#			$stderr.puts "Dependency '#{n}' already got."
			return
		end
		
		if @name2file.has_key?(n) 
#			$stderr.puts "Dependency '#{n}' Found!"
			@required.push @name2file[n]
			parse_file(@name2file[n]);
		else
			@notfound.push n if not @notfound.include? n
#			$stderr.puts "Dependency '#{n}' not found."
		end
	end
	
	def parse_file(file)
		# TODO: se e' directory falli tutti
		File.open(file,'r') do |f| parse_io(f) end
	end
	
	def parse_io(io)
		# variables used 
		variables = Array.new
		# functions defined in file
		functions = Array.new
		
		# add built-ins
		functions.push(*['find','if','eye','fprintf','sum','axis',
			'norm','error','plot','pause','sprintf'])
		
		doRequires = true
		io.each_line do |l|
			if false
			if l =~ /\s*function .*=\s*(\w+)/  ||
			   l =~ /\s*function\s*(\w+)/
				# puts "Defined function #{$1}"
				functions.push $1
			end
			l.split(';').each do |statement|
				if statement =~ %r|\s*(\w+)\s*=| ||
				   statement =~ %r|(\w+)(\(.*\))*\s*=|
					# puts "Is #{$1} a variable in this? #{statement}"
					variables.push $1
				end
			end
			
			# TODO: ignore after comments
			# TODO: parse strings (ex: fprintf('False(%s,%s;%s)'))
			l.scan(/(\w+)\(/) { |a| f=a[0];
				need_function(f) unless variables.include?(f) or
					functions.include?(f)
			}

			if l =~ /^\s*%\s*Requires:\s*(.*)$/
				$1.split(",").each do |x| 
					need_function(x.strip) 
				end
			end
			end
			l.scan(/([A-Za-z_]\w*)/) { |a| f=a[0];
				need_function(f)			
			}
		end
	end
end

def print_usage(error)
	puts "create_package [--directory <dir1>,<dir2>,...]*" 
	puts help
end

def go
	version = 0.1;
	
	opts = GetoptLong.new(
		[ "--directory", "-d",   GetoptLong::REQUIRED_ARGUMENT ],
		[ "--help",      "-h",   GetoptLong::NO_ARGUMENT ],
		[ "--version",   "-v",   GetoptLong::NO_ARGUMENT ]
	)
	 	 

	directories = Array.new
	begin
		ndirectory = 0;
		opts.each do |opt, arg|
			case opt
				when "--directory"
					arg.split(",").each do |single|
						directories.push(File.expand_path(single))  
					end
					ndirectory += 1
				 when "--help"
					  print_usage(0); exit(0);
				 when "--usage"
					  print_usage(0); exit(0);
				 when "--version"
					  print "csvt, version ", version, "\n"
					  exit(0)
			end
		end

	rescue 
		print_usage(1)
	end
		
	directories.push File.expand_path('.') if directories.size == 0
	
	db = DB.new
	directories.each do |d|
		db.explore_directory(d)
	end

	files = ARGV;
	
	if files.empty?
		db.parse_io($stdin);
	else
		files.each do |x|
			db.required.push(Filename.expand(x))
			db.parse_file(x)
		end
	end
	
	db.required.each do|r|
		puts r
	end

#	$stderr.puts "Found #{db.required.size} deps. Not found: " + 
#		db.notfound.join(", ") + "."
	
end

go
