#!/usr/bin/env ruby

require 'rsm_sm_loop'

	
	if ARGV.size < 3
		puts "sm <SCANMATCHER> '[]' <input> <output> "
		exit 1
	end

	scan_matcher = eval ARGV[0]
	scan_list = eval ARGV[1]

	puts "Scan_list = #{scan_list}"

# Read from standard input if no arguments are passed
#io = ARGV.size>0 ? File.open(ARGV[0]) : $stdin 

	input = File.open ARGV[2]
	output = File.open ARGV[3], "w"

	params = standard_parameters
	
#	begin
		scan_matching(scan_matcher,scan_list,input,output,params)
	# rescue => ex
	# 	puts "bam"
	# 	puts ex, ex.backtrace
	# end
	
	

