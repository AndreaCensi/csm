#!/usr/bin/env ruby
require 'rubygems'
require 'json'
require 'fileutils'

$LOAD_PATH << File.expand_path(File.dirname(__FILE__))
require 'cmd_utils'

FIG2PICS = find_cmd('fig2pics.rb')
PDF2SWF = find_cmd('pdf2swf')
GS = find_cmd('gs')

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
	execute_cmd "#{FIG2PICS} -f '-D #{opt_depths}' -o #{filename} #{other} #{fig}"
	filenames.push filename
end


filenames = filenames.join(' ')
cmd = "#{GS} -dNOPAUSE -sDEVICE=pdfwrite -sOUTPUTFILE=#{output_pdf} -dBATCH #{filenames}"
execute_cmd cmd

cmd = "#{PDF2SWF} -L /usr/local/share/swftools/swfs/swft_loader.swf "+
      " -B /usr/local/share/swftools/swfs/keyboard_viewer.swf " +
      " #{output_pdf} -o #{output_swf}"
execute_cmd cmd