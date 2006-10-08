
require 'mbicp_tro_utils'

scans = read_log(File.open('laserazosSM3.off'))

out = File.open('laserazosSM3.log','w')

scans.each do |s|
	out.puts s.to_carmen
end