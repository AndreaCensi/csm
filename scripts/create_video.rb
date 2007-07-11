#!/usr/bin/env ruby
require 'rubygems'
require 'json'
require 'fileutils'
require 'optparse'

$LOAD_PATH << File.expand_path(File.dirname(__FILE__))
require 'cmd_utils'

def show_help()
	cmd = File.basename($0)
	description= <<-EOF
#{cmd}: creates an animation from a JSON file describing the animation.

Usage:

	#{cmd} [options] file.desc

Example animation description file:

	{ 
		"fig": "figure.fig", 
		"common_depths": [ 100] 
		"frames": [ 
			{ "depths": [ 1,2 ] }, 
			{ "depths": [ 3,4 ] } 
		], 
	}

This is a movie created from "figure.fig", consisting of
two frames. Depth 100 is shown in both. Frame#1 is created
from depth 1,2 and frame #2 is created from depths 3,4.

EOF
	$stderr.puts description
end

FIG2PICS = find_cmd('fig2pics.rb')
PDF2SWF = find_cmd('pdf2swf')
GS = find_cmd('gs')

	verbose = false
	opt = OptionParser.new do |opts|
		opts.on("-v", "--verbose", " be verbose") do |s| verbose=true; end
		opts.on_tail("-h", "--help", "Show this message") do 
			show_help()
			$stderr.puts "Options: "
			$stderr.puts opts
			exit 0
		end
	end
	begin opt.parse!
	rescue OptionParser::InvalidOption=>e
		$stderr.puts e
		$stderr.puts opt
		exit
	end

if(ARGV.size != 1) 
	show_help()
	exit -1
end

spec = ARGV[0]

spec_content = File.open(spec).read 

h = JSON.parse(spec_content)

fig = File.join(File.dirname(spec), h['fig'])
frames = h['frames'].map {|f| f['depths'] + h['common_depths']}

dir=File.dirname(spec)
basename=File.basename(spec,".fig.desc").sub(/^\.\//,"");
output_pdf = "#{dir}/#{basename}.pdf"
output_swf = "#{dir}/#{basename}.swf"
temp_dir = "#{dir}/#{basename}"
FileUtils.mkdir_p(temp_dir) if not File.exists? temp_dir

filenames = []
frames.each_with_index do |depths, i |
	depths = depths.map {|x| x == -1 ? nil : x}.compact.uniq
	opt_depths = "+" + depths.join(',')
	filename = sprintf("#{temp_dir}/video%04d.pdf", i)
	other = ARGV[1, ARGV.size].map{|x|x.inspect }.join(' ')
	execute_cmd_verb("#{FIG2PICS} -f '-D #{opt_depths}' -o #{filename} #{other} #{fig}",verbose)
	filenames.push filename
end


filenames = filenames.join(' ')
cmd = "#{GS} -dNOPAUSE -sDEVICE=pdfwrite -sOUTPUTFILE=#{output_pdf} -dBATCH #{filenames}"
execute_cmd_verb(cmd, verbose)

cmd = "#{PDF2SWF} -L /usr/local/share/swftools/swfs/swft_loader.swf "+
      " -B /usr/local/share/swftools/swfs/keyboard_viewer.swf " +
      " #{output_pdf} -o #{output_swf}"
execute_cmd_verb(cmd, verbose)
