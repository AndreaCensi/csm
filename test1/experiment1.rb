#!/usr/bin/env ruby
require 'optparse'
require 'fileutils'
$LOAD_PATH << File.expand_path(File.dirname(__FILE__))
# export PATH=$PATH:~/icra07pis/src/libraries/fig/
scripts_dir = File.expand_path File.join(File.dirname(__FILE__), '../scripts')
$LOAD_PATH << File.expand_path(scripts_dir)
require 'cmd_utils'
require 'hash_options'

additional_paths = ['../sm/', '../scripts'].
	map {|x| File.join(File.dirname($0), x) }

RAYTRACER   = find_cmd('raytracer',      additional_paths)
LD_DRAW     = find_cmd('ld_draw',        additional_paths)
JSON_PIPE   = find_cmd('json_pipe',      additional_paths)
LD_NOISE    = find_cmd('ld_noise',       additional_paths)
LD_SLIP     = find_cmd('ld_slip',       additional_paths)
SM1         = find_cmd('sm1',            additional_paths)
JSON2MATLAB = find_cmd('json2matlab.rb', additional_paths)
#FIGMERGE    = find_cmd('figmerge',       additional_paths)

CONFIG = {}
def set(name, value) CONFIG[name.to_s] = value end
def cfg(name) val = CONFIG[name.to_s]; 
	if not val 
		$stderr.puts "No value for #{name}"
		exit -1
	end
	val
end
=begin

scan1 = good perfect map without noise at pose 1

scan2 = sensor scan at pose2
scan3 = sensor scan at pose1

=end
set :out,             "out/"
set :num,             3
set :fig,             'square.fig'
set :first_pose,      "5 5 0"
set :second_pose,     "5.1 5.2 0.0"
set :scan1_noise,           "-seed 10 -sigma 0.01"
set :scan2_noise,           "-seed 11 -sigma 0.01"
set :scan2_slip,            "-seed 12 -sigma_xy 0.05 -sigma_theta_deg 3 "
set :raytracer_args,  "-max_reading 80"
set :raytracer_map,   "-nrays 359 -fov_deg 360"
set :raytracer_scan,  "-nrays 181 -fov_deg 180"
set :sm1_config,      "sm1.config"
set :sm1_journal,     "jj.txt"

	opt = OptionParser.new do |opts|
#		opts.banner = "Usage: maruku [options] [file1.md [file2.md ..."
#		opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
#			MaRuKu::Globals[:verbose] = v end
		opts.on("--config CONFIG", "Options to pass to fig2dev") do |s|
			CONFIG.read_conf(File.open(s), File.dirname(s))
		end
		opts.on("--config_dump", "Dumps config to stdout") do |s|
			puts CONFIG.config_dump
			exit 0
		end
	end

	begin  opt.parse!
	rescue OptionParser::InvalidOption=>e
		$stderr.puts e
		$stderr.puts opt
		exit -1
	end

	dir = cfg :out
	execute_cmd "mkdir -p #{dir}"
	File.open("#{dir}/ex1.config",'w') do |f| f.puts CONFIG.config_dump end

# Create perfect map
execute_cmd "echo #{cfg :first_pose} | "+
	"#{RAYTRACER} -fig #{cfg :fig} #{cfg :raytracer_args} #{cfg :raytracer_map} -out #{dir}/scan_map.txt"
	
execute_cmd "echo #{cfg :first_pose} | "+
	"#{RAYTRACER} -fig #{cfg :fig} #{cfg :raytracer_args} #{cfg :raytracer_scan} -out #{dir}/scan1.txt"
execute_cmd "echo #{cfg :second_pose} | "+
	"#{RAYTRACER} -fig #{cfg :fig} #{cfg :raytracer_args} #{cfg :raytracer_scan} -out #{dir}/scan2.txt"

# Perfect map
execute_cmd "#{JSON_PIPE} -n #{cfg :num} < #{dir}/scan_map.txt | "+
	"#{LD_NOISE} | #{LD_SLIP} > #{dir}/scan_map_rep.txt"
	
# no slip to scan1
execute_cmd "#{JSON_PIPE} -n #{cfg :num} < #{dir}/scan1.txt | "+
	"#{LD_NOISE} #{cfg :scan1_noise} | #{LD_SLIP}  > #{dir}/scan1_noise.txt"

# odometry error on scan2
execute_cmd "#{JSON_PIPE} -n #{cfg :num} < #{dir}/scan2.txt | "+
	"#{LD_NOISE} #{cfg :scan2_noise} | #{LD_SLIP} #{cfg :scan2_slip} > #{dir}/scan2_noise.txt"

execute_cmd "#{JSON2MATLAB} #{dir}/scan_map.txt"
execute_cmd "#{JSON2MATLAB} #{dir}/scan1.txt"
execute_cmd "#{JSON2MATLAB} #{dir}/scan2.txt"
execute_cmd "#{JSON2MATLAB} #{dir}/scan1_noise.txt"
execute_cmd "#{JSON2MATLAB} #{dir}/scan2_noise.txt"

do_journal = (cfg :sm1_journal).size > 0

quiet = " 2> /dev/null "
# first, localization
journal = do_journal ?  "-file_jj #{dir}/loc_journal.txt" : ""
execute_cmd "#{SM1} -file1 #{dir}/scan_map_rep.txt -file2 #{dir}/scan2_noise.txt "+
	"#{journal} -config #{cfg :sm1_config} #{quiet} > #{dir}/loc_results.txt "

execute_cmd "#{JSON2MATLAB} #{dir}/loc_results.txt"

# then, scan matching
journal = do_journal ?  "-file_jj #{dir}/sm_journal.txt" : ""
execute_cmd "#{SM1} -file1 #{dir}/scan1_noise.txt -file2 #{dir}/scan2_noise.txt "+
	"#{journal} -config #{cfg :sm1_config} #{quiet} > #{dir}/sm_results.txt "

execute_cmd "#{SM1} -config #{cfg :sm1_config} -config_dump > #{dir}/sm1.config"

execute_cmd "#{JSON2MATLAB} #{dir}/sm_results.txt"

# drawing
#$LD_DRAW -config scan1.config < $dir/scan1.txt > $dir/scan1.fig
#$LD_DRAW -config scan2.config < $dir/scan2.txt > $dir/scan2.fig

#$FIGMERGE $fig $dir/scan1.fig > $dir/scan1-map.fig
#$FIGMERGE $fig $dir/scan2.fig > $dir/scan2-map.fig
#$FIGMERGE $fig $dir/scan1.fig $dir/scan2.fig > $dir/complete.fig
