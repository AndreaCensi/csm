require 'sm_icp'
require 'sm_gpm'

class GPM_then_ICP
	attr_accessor :params
	
	def initialize
		@params = Hash.new
		@journal = nil
	end
	
	def name
		'GPM+ICP'
	end
	
	def journal_open(file)
		@journal = file
	end
	
	def scan_matching
		gpm = Sm::GPMC.new
		gpm.params = params
		if not @journal.nil?
			gpm.journal_open(@journal+"_gpm")
		end
		
		res_gpm =gpm.scan_matching 
		
		if not res_gpm[:valid]
			$stderr.puts "GPM was not successful"
			return res_gpm
		end
		gpm_x = res_gpm[:x]
			
		p res_gpm
		puts "GPM_then_ICP: gpm res is #{pv(gpm_x)}"
		icpc = Sm::ICPC.new
		icpc.params = params;
		icpc.params[:firstGuess] = gpm_x

		if not @journal.nil?
			puts "opening"
			icpc.journal_open(@journal+"_icp")
		end
		
		res = icpc.scan_matching;

		res
	end
end