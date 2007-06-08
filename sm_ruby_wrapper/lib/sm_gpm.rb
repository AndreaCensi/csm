require 'sm'		
require 'sm_icp'

require 'rsm'

module Sm
	
	class GPMC
		include MathUtils	
		include Math
	
		attr_accessor :params
	
		def initialize
			@params = standard_parameters
		end
	
		def name 
			"GPMC"
		end

		def scan_matching
		
			Sm.put_params_in_c_structures(params)
		
			Sm::rb_sm_gpm()
		
			res = Sm.get_result_from_c_structures()
		
			Sm::rb_sm_cleanup()
		
			res
		end
	
	
		def journal_open(filename)
			Sm::rb_sm_init_journal(filename)
		end
	end

end