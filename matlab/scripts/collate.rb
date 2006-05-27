#!/usr/bin/env ruby

block = Array.new
block_indent = '';

$stdin.each_line do |l|
	if l =~ /^(\s*)%%(.*)$/
		if block.size == 0
			block_indent = $1;
		end
		block.push $2
	else
		# flush block
		if block.size > 0
			s = block_indent + "%% ";
			block.each do |m|
				s += (m.strip.size>0?m:'\\par');
			end
			$stdout.puts s
			block.clear
		end
		$stdout.puts l
	end
end
