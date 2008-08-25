
require 'ostruct'
require 'optparse'

class Options
	Option = Struct.new(:name, :default, :description, :value)
		
	def initialize
		@hash = {}
		@order = []
	end
	
	def add(name, default, description)
	  if name.class == String
	    name = name.to_sym
	  end
	  if name.class != Symbol
	    raise "This: #{name.inspect} should be a String or Symbol"
	  end
	  
		@hash[name] = Option.new(name, default, description, nil)
		@order.push @hash[name]
	end

	def add_required(name, description)
	  add(name, nil, description)
	end
	
	
	def populate(opts)

		opts.on("--help", "shows help") do
		  $stderr.puts opts.help
		  exit 0
		end

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
				rescue SyntaxError => ex
				rescue => ex
					end
				
				@hash[o.name].value = value
			end
		end
			
	end
	
	# Create an ostruct from the data
	def get_ostruct
		r = {}
		@hash.each do |k, v| 
			r[k] = v.value || v.default
		end
 		OpenStruct.new r
	end
	
	# Warn if some required parameter was not set
	# (default and value are nil)
	def warn_required
	  res = false
		@order.each do |v| 
			 if v.value.nil? && v.default.nil?
			    $stderr.puts "Required option '#{v.name}' not set."
			    res = true
			 end
		end	  
		res
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
	RegLoad    = /^\s*(?:<|source)\s*(.*)$/

	def load_config_from_file(file)
		load_config_from_string(File.open(file).read, File.dirname(file))
	end
	
	def load_config_from_string(string, dir)
		string.split("\n").each do |line|
			next if (line.strip.size == 0) || line =~ RegComment
			if m = RegLoad.match(line)
				filename = m[1]
				filename = File.expand_path(File.join(dir, filename))
				load_config_from_file(filename)
			elsif m = RegOption.match(line)
				name = m[1].to_sym
				value = m[2].strip

				begin
					value = eval(value)
				rescue ScriptError,StandardError
				end
				
				if o = @hash[name]
					o.value = value
#					$stderr.puts "#{name} = #{value}"
				else
					$stderr.puts "Unknown key #{name.inspect} (#{@hash.keys.inspect})"
				end
			else
				$stderr.puts "Line #{line.inspect} is malformed"
				exit -1
			end
		end
	end
	
	def parse_cmd_line!
		opt = OptionParser.new do |opts|
			self.populate(opts)
		end

		begin  opt.parse!
		rescue OptionParser::InvalidOption=>e
			$stderr.puts e
			$stderr.puts opt
			exit -1
		end
	end
	

end