#!/usr/bin/env ruby
require 'logreader'
require 'icp'

log = LogReader.read_log($stdin)

def next_ld(log)
#	do
#		e = log.shift
#		if e.kind_of? LaserData then e
#	until e.kind_of?(LaserData)
	log.shift
end

puts "Start processing: #{log.size}"

ld_ref = next_ld(log)
until log.empty?
	ld_new = next_ld(log)

	icp = ICP.new(ld_ref, ld_new);
	icp.scan_matching
end
