#!/usr/bin/env ruby
require 'optparse'
require 'fileutils'
# export PATH=$PATH:~/icra07pis/src/libraries/fig/
require 'cmd_utils'
require 'hash_options'

additional_paths = ['../sm/', '../matlab_new/'].
	map {|x| File.join(File.dirname($0), x) }

RAYTRACER   = find_cmd('raytracer',      additional_paths)
LD_DRAW     = find_cmd('ld_draw',        additional_paths)
#FIGMERGE    = find_cmd('figmerge',       additional_paths)
JSON_PIPE   = find_cmd('json_pipe',      additional_paths)
LD_NOISE    = find_cmd('ld_noise',       additional_paths)
LD_SLIP     = find_cmd('ld_slip',       additional_paths)
SM1         = find_cmd('sm1',            additional_paths)
JSON2MATLAB = find_cmd('json2matlab.rb', additional_paths)

CONFIG = {}
def set(name, value) CONFIG[name.to_s] = value end
def cfg(name) CONFIG[name.to_s] end
	
set :out,             "out/"
set :num,             3
set :fig,             'square.fig'
set :pose1,           "5 5 0"
set :pose2,           "5.1 5.2 0.0"
set :scan2_noise,     "-sigma 0.01"
set :scan2_slip,      "-sigma_xy 0.05 -sigma_theta_deg 0 "
set :raytracer_args,  "-max_reading 80"
set :raytracer_scan1, "-nrays 359 -fov_deg 360"
set :raytracer_scan2, "-nrays 181 -fov_deg 180"
set :sm1_config,      "sm1.config"


	opt = OptionParser.new do |opts|
#		opts.banner = "Usage: maruku [options] [file1.md [file2.md ..."
#		opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
#			MaRuKu::Globals[:verbose] = v end
		opts.on("--config CONFIG", "Options to pass to fig2dev") do |s|
			CONFIG.read_conf(File.open(s))
		end
	end

	begin  opt.parse!
	rescue OptionParser::InvalidOption=>e
		$stderr.puts e
		$stderr.puts opt
		exit -1
	end

	dir = cfg :out
	execute_cmd "cp #{cfg :sm1_config} #{dir}/sm1.config"
	File.open("#{dir}/ex1.config",'w') do |f| f.puts CONFIG.config_dump end

FileUtils.mkdir_p dir

execute_cmd "echo #{cfg :pose1} | "+
	"#{RAYTRACER} -fig #{cfg :fig} #{cfg :raytracer_args} #{cfg :raytracer_scan2} -out #{dir}/scan1.txt"
execute_cmd "echo #{cfg :pose2} | "+
	"#{RAYTRACER} -fig #{cfg :fig} #{cfg :raytracer_args} #{cfg :raytracer_scan1} -out #{dir}/scan2.txt"

execute_cmd "#{JSON_PIPE} -n #{cfg :num} < #{dir}/scan1.txt | "+
	"#{LD_NOISE} | #{LD_SLIP} > #{dir}/scan1_noise.txt"
execute_cmd "#{JSON_PIPE} -n #{cfg :num} < #{dir}/scan1.txt | "+
	"#{LD_NOISE} #{cfg :scan2_noise} | #{LD_SLIP} #{cfg :scan2_slip} > #{dir}/scan2_noise.txt"

#$LD_DRAW -config scan1.config < $dir/scan1.txt > $dir/scan1.fig
#$LD_DRAW -config scan2.config < $dir/scan2.txt > $dir/scan2.fig

#$FIGMERGE $fig $dir/scan1.fig > $dir/scan1-map.fig
#$FIGMERGE $fig $dir/scan2.fig > $dir/scan2-map.fig
#$FIGMERGE $fig $dir/scan1.fig $dir/scan2.fig > $dir/complete.fig

execute_cmd "#{SM1} -file1 #{dir}/scan1_noise.txt -file2 #{dir}/scan2_noise.txt "+
	"-config #{cfg :sm1_config} > #{dir}/results.txt "

# make $dir/results.m
# make $dir/scan1.m
# make $dir/scan1_noise.m
# make $dir/scan2.m
# make $dir/scan2_noise.m


