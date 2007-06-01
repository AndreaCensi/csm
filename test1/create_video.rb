#!/usr/bin/env ruby
require 'rubygems'
require 'json'
require 'fileutils'

def execute_cmd(m)
	puts "$ " + m 
	system m
	if $?.exitstatus != 0
		puts "\n\n\t command:\n\n\t#{m}\n\n\tFAILED\n\n"
		exit($?.exitstatus)
	end
end

def search_cmd(program)
	path = ENV['PATH'].split(":") 
	path.push File.dirname($0)
	path.push Dir.pwd
	for dir in path
		p = File.join(dir, program)
		if File.exists? p
			$stderr.puts "Using #{p.inspect}"
			return p
		end
	end
	puts "\n\n Could not find program #{program.inspect}"
	exit -1
end

FIG2PICS = search_cmd('fig2pics.rb')

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
	cmd = "#{FIG2PICS} -f '-D #{opt_depths}' -o #{filename} #{other} #{fig}"
	execute_cmd cmd
	filenames.push filename
end


filenames = filenames.join(' ')
cmd = "gs -dNOPAUSE -sDEVICE=pdfwrite -sOUTPUTFILE=#{output_pdf} -dBATCH #{filenames}"
execute_cmd cmd

cmd = "pdf2swf -L /usr/local/share/swftools/swfs/swft_loader.swf "+
      " -B /usr/local/share/swftools/swfs/keyboard_viewer.swf " +
      " #{output_pdf} -o #{output_swf}"
execute_cmd cmd