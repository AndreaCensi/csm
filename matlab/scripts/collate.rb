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
			sub_blocks = Array.new
			sub_blocks.push Array.new
			
			block.each do |m|
				if m.strip.size == 0
					sub_blocks.push Array.new
				else
					sub_blocks.last.push m
				end
			end
			
			sub_blocks.each do |s|
				if s.size>0
					$stdout.puts block_indent + "%% " + s.join;
				end
			end
			
			block.clear
		end
		# and write last line
		$stdout.puts l
	end
end
