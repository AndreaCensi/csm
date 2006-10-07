require 'sm'
require 'structures'
require 'journal'
		
require 'icpc_wrap'

class GPMC
	include MathUtils	
	include Math
	include Sm
	
	attr_accessor :params
	
	def initialize
		@params = standard_parameters
	end
	
	def name 
		"GPMC"
	end

	def scan_matching
		
		put_params_in_c_structures(params)
		
		rb_sm_gpm()
		
		res = get_result_from_c_structures()
		
		rb_sm_cleanup()
		
		puts "gpm_res: #{res[:x]}"
		res
	end
	
	
	def journal_open(filename)
		rb_sm_init_journal(filename)
	end
end