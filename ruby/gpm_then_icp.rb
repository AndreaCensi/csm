require 'icpc_wrap'
require 'icp'


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
		gpm = GPM.new
		gpm.params = params
		if not @journal.nil?
			gpm.journal_open(@journal+"_gpm")
		end
		
		gpm_x = (gpm.scan_matching)[:x]
			
		icpc = ICPC.new
		icpc.params = params;
		icpc.params[:firstGuess] = gpm_x

		if not @journal.nil?
			puts "opening"
			icpc.journal_open(@journal+"_icp")
		end
		
		return icpc.scan_matching
	end
end