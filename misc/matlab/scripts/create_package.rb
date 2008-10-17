#!/usr/bin/env ruby

require 'getoptlong'

class DB
	attr_accessor :required
	
	def initialize()
		@name2file = Hash.new
		@required = Array.new
	end

	def explore_directory(d)
		pwd = Dir.pwd
		Dir.chdir(d)
		Dir.glob('**/*.m').each do |x| 
			name = File.basename(x,'.m');
			@name2file[name]=File.join(d, x);
		#	puts "Found function #{name} in file #{x}."
		end
		Dir.chdir(pwd)
	end
	
	def need_function(n)
		return if @required.include?(@name2file[n])
	
		if @name2file.has_key?(n) 
			@required.push @name2file[n]
			parse_file(@name2file[n]);
		else
			$stderr.puts "Dependency '#{n}' not found."
		end
	end
	
	def parse_file(file)
		# TODO: se e' directory falli tutti
		File.open(file,'r') do |f| parse_io(f) end
	end
	
	def parse_io(io)
		io.each_line do |l|
			if l =~ /^\s*%\s*Requires:\s*(.*)$/
				$1.split(",").each do |x| 
					need_function(x.strip) 
				end
			end
		end
	end
end

def print_usage(error)
	puts "create_package [--directory <dir1>,<dir2>,...]*" 

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
			db.parse_file(x)
		end
	end
	
	db.required.each do|r|
		puts r
	end
	
end

go
