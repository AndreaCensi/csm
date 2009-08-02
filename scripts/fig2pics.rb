#!/usr/bin/env ruby

# 2009-05-03: starting tagging versions before copying over
#
# 2007-08-22: sostituto \usepackage{times} a \usepackage{ae,aecompl}
#

require 'tempfile'
require 'optparse'
require 'fileutils'
require 'pathname'

$LOAD_PATH << File.expand_path(File.dirname(__FILE__))
require 'cmd_utils'

SCALE=4

BBOX = /^%%BoundingBox: (\d+) (\d+) (\d+) (\d+)$/

def eps_get_bounds(epsfile)
	File.open(epsfile) do |f|
		f.each do |line|
			if line =~ BBOX then
				bounds = [$1, $2, $3, $4].map{|x| x.to_i}
				puts "Found #{line}" + bounds.inspect
				f.close
				return bounds
			end
		end
	end
	nil
end

def eps_substitute_first_line(input, output, line_reg, replacement)
	done = false
	input.each do |line|
		if not done and line =~ line_reg
			output.puts replacement
		else
			output.puts line
		end
	end
end

def change_bounds(eps, limits) 
	bounds   = eps_get_bounds(eps)
	if not bounds
		$stderr.puts "Could not find bounding box in file #{eps.inspect}"
		exit 1
	end
	
	corner_x = bounds[0]
	corner_y = bounds[1]
	width    = bounds[2] - bounds[0]
	height   = bounds[3] - bounds[1]
	
	new_bounds = [
		corner_x + width * limits[0],
		corner_y + height * limits[1],
		corner_x + width * limits[2],
		corner_y + height * limits[3]]
		
	$stderr.puts  "New bounds: #{new_bounds.inspect}"
	temp_file = Tempfile.new('change_bounds')
	eps_substitute_first_line(File.open(eps), temp_file, BBOX,
		'%%BoundingBox: '+new_bounds.join(' '))
	temp_file.close
	FileUtils.cp(temp_file.path, eps)
	temp_file.unlink
end

def fig2pics()
	opt_preamble = nil
	opt_fig2dev = nil
	opt_output = nil
	opt_bounds = nil
	opt_verbose = false
	opt_debug = false
	
	opt = OptionParser.new do |opts|
#		opts.banner = "Usage: maruku [options] [file1.md [file2.md ..."
#		opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
#			MaRuKu::Globals[:verbose] = v end
		opts.on("-v", "--verbose", "Be verbose") do opt_verbose = true; end

		opts.on("-p", "--preamble PREAMBLE", "LaTeX preamble file") do |s|
			opt_preamble = s
			if ! File.exists?(opt_preamble) 
				puts "Error: file #{opt_preamble.inspect} does not exist."
				exit 2
			end
			$stderr.puts "Using LaTeX preamble #{s}."
		end

		opts.on("-f", "--fig-options OPTIONS", "Options to pass to fig2dev") do |s|
			opt_fig2dev = s
		end

		opts.on("-o", "--output FILE", "Output filename") do |s|
			opt_output = s
		end

		opts.on("-b", "--bbox BBOX", "Bounding box (4 numbers like '0 0 1 0.50')") do |s|
			opt_bounds = s.strip.split.map{|x|x.to_f}.compact
			if opt_bounds.size != 4
				$stderr.puts "Malformed bounds: #{s.inspect} -> #{opt_bounds.inspect}"
				exit 4
			end
			if (opt_bounds[0] >= opt_bounds[2]) || (opt_bounds[1] >= opt_bounds[3]) ||
				 opt_bounds.any?{|x| x < 0 || x > 1 } then
				$stderr.puts "Bounds #{opt_bounds.inspect} should respect constraints"
				exit 4
			end
		end

		opts.on_tail("-h", "--help", "Show this message") do
			puts opts
			exit
		end

	end

	begin 
	opt.parse!
	rescue OptionParser::InvalidOption=>e
		$stderr.puts e
		$stderr.puts opt
		exit
	end

	if ARGV.size == 0 
		puts "Error: need filename"
		exit 1
	end

	input=ARGV[0]
	if !File.exists?(input) 
		puts "Error: file #{input} does not exist."
		exit 1
	end

# 
# if ARGV.size >= 5
# 	puts "Error: fig2pics.rb <file.fig> [<preamble.tex>] [params to fig2dev] '0 0 0.4 0'"
# 	exit 3
# end

programs=["latex", "dvips",  "fig2dev", "epstopdf"];

programs.each do |p| 
	abs = `which #{p}`.chop
	$stderr.puts "Using " + abs.inspect if opt_debug
	if not FileTest.exists?(abs) then	
		$stderr.puts "Error: command #{p} not available (abs=#{abs.inspect})"
		exit
	end
end

dir=File.dirname(input)

basename=File.basename(input,".fig").sub(/^\.\//,"");
$stderr.puts "dir #{basename}" if opt_debug
$stderr.puts "basename #{basename}" if opt_debug
outpng="#{dir}/#{basename}.png"
outpdf="#{dir}/#{basename}.pdf"
temp="#{dir}/#{basename}_tmp"

slideprefix="#{dir}/#{basename}_slide"
slide="#{dir}/#{basename}_slide.tex"
slidedvi="#{dir}/#{basename}_slide.dvi"
slideps="#{dir}/#{basename}_slide.ps"
slidepdf="#{dir}/#{basename}_slide.pdf"

abs_dir = Pathname.new("#{dir}").realpath
absolute_img = abs_dir + "#{basename}_tmp.eps"

execute_cmd "fig2dev -L pstex_t  #{opt_fig2dev} -p #{absolute_img} #{input} #{temp}.tex"
execute_cmd "fig2dev -L pstex    #{opt_fig2dev}    #{input} #{temp}.eps"

preamble_text = opt_preamble ? "\\input{#{opt_preamble}}" : "%"

slidecontent=<<EOF
		\\documentclass{article}
		\\usepackage{amsmath}
		\\usepackage{amsthm}
		\\usepackage{amssymb}
		\\usepackage{color}
		\\usepackage{epsfig}
		\\usepackage[T1]{fontenc}
		\\usepackage{times}
%  \\usepackage{ae,aecompl,aeguill}
		#{preamble_text} 
		\\begin{document}\\thispagestyle{empty}
		\\input{#{abs_dir}/#{basename}_tmp.tex}
		\\end{document}
EOF

File.open(slide,"w") do |f|
	f.puts slidecontent 
end

execute_cmd	"latex -output-directory=#{abs_dir} #{slide}"
execute_cmd	"dvips -q -Ppdf -E #{slidedvi} -o #{slideps}"

if opt_bounds then change_bounds(slideps, opt_bounds) end

if opt_output then outpdf = opt_output end


execute_cmd("epstopdf #{slideps} --outfile=#{outpdf} --debug", exit_on_error=true)

execute_cmd  "rm -f #{slide} #{slidedvi} #{slideps} #{temp}.* #{slideprefix}.*"

#execute_cmd	"pstoimg -antialias -aaliastext -scale #{SCALE} -out #{outpng} #{slideps}"
#	epstopdf #{figureeps} $WHERE/figure-$basename.eps
#execute_cmd     "convert -density 600x600 #{slidepdf} -resize 640 #{outpng}"
#execute_cmd "eps2eps #{basename}_tmp.eps temp.eps"
#execute_cmd "epstopdf  temp.eps "
#execute_cmd "cp  temp.pdf #{basename}_tmp.pdf "
end


fig2pics





