
require 'ostruct'

class Options
	Option = Struct.new(:name, :default, :description, :value)
		
	def initialize
		@hash = {}
		@order = []
	end
	
	def add(name, default, description)
		@hash[name] = Option.new(name, default, description, nil)
		@order.push @hash[name]
	end
	
	
	def populate(opts)
		#		opts.banner = "Usage: maruku [options] [file1.md [file2.md ..."
		#		opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
		#			MaRuKu::Globals[:verbose] = v end

		opts.on("--config FILE", "Load config from file") do |file|
			load_config_from_file(file)
		end

		opts.on("--config_dump", "Dump config on stdout") do 
			$stdout.puts self.config_dump
			exit 0
		end

		@order.each do |o|
			s = o.default.inspect.ljust(25)
			opts.on("--#{o.name} VALUE", "#{s} #{o.description}") do |s|
				value = s
				begin value = eval(s) 
				rescue 
					end
				
				@hash[o.name].value = value
			end
		end
			
	end
	
	def get_ostruct
		r = {}
		@hash.each do |k, v| 
			r[k] = v.value || v.default
		end
 		OpenStruct.new r
	end
	
	def config_dump
		s = ""
		max_size = @hash.keys.map{|x|x.to_s.size}.max + 2
		for o in @order
			s += o.name.to_s.ljust(max_size) +  (o.value || o.default).inspect + "\n"
		end
		s
	end
	
	RegComment = /^\s*\#/ 
	RegOption  = /^\s*(\w+)\s*=?\s*(.+)$/ 

	def load_config_from_file(file)
		load_config_from_string(File.open(file).read, File.dirname(file))
	end
	
	def load_config_from_string(string, dir)
		string.split("\n").each do |line|
			next if (line.strip.size == 0) || line =~ RegComment
			if m = RegOption.match(line)
				name = m[1].to_sym
				value = m[2].strip

				begin
					value = eval(value)
				rescue 
				end
				
				if o = @hash[name]
					o.value = value
#					$stderr.puts "#{name} = #{value}"
				else
					$stderr.puts "Unknown key #{name.inspect} (#{@hash.keys.inspect})"
				end
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


end