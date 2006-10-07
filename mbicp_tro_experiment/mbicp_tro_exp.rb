#!/usr/bin/env ruby

require 'benchmark'
require 'mbicp_tro_utils'


def main(scans, klass)		
	
	max_displacement = [
		Vector[0.05, 0.05, deg2rad( 2.0)].col,
		Vector[0.10, 0.10, deg2rad( 4.0)].col,
		Vector[0.15, 0.15, deg2rad( 8.6)].col,
#		Vector[0.20, 0.20, deg2rad(17.2)].col,
#		Vector[0.20, 0.20, deg2rad(34.3)].col,
#		Vector[0.20, 0.20, deg2rad(45.0)].col
];
	
	repetitions_per_scan = 1;	
	
	# Use a known seed for repeatability of the experiments
	rng = Rng.alloc(GSL::Rng::MT19937, 42)
	sigma = 0.01;
	sigma = 0;
	
	f = File.open("results.txt",'w')
	failed_codes = Array.new
	max_displacement.each_index do |i|
		hist = Histogram.alloc([0, 0.001, 0.005, 0.01, 0.05, 1000]);
		hist_iterations = Histogram.alloc(20, [0, 20]);
		codes = Array.new; hist.bins.times do codes.push Array.new end
			
		failed = 0;
		puts "Experiment #{i}"
		scans.each_index do |s| repetitions_per_scan.times do |n|
			disp = random_displacement(max_displacement[i],rng);
			
			code = "#{i}-#{s}-#{n}"

			if not ARGV.empty?
				next if not ARGV.include?(code)
			end
			
			sm = klass.new
			
			if ARGV.include?(code)
				sm.journal_open("sm_#{sm.name}_#{code}.txt")
			end
			
			puts "#{code} displacement: #{pv(disp)}"
			sm.params = standard_parameters
			sm.params[:epsilon_xy]=  0.001 / 10
			sm.params[:epsilon_theta]= 0.001 / 10
			
			sm.params[:maxAngularCorrectionDeg]= rad2deg(max_displacement[i][2])*1.3
			sm.params[:maxLinearCorrection]=  max_displacement[i][0]*1.3;
			sm.params[:laser_ref] =  scans[s]
			sm.params[:laser_sens] =  scans[s].deep_copy
			sm.params[:laser_sens].add_noise!(sigma, rng)
			sm.params[:firstGuess] = disp
			sm.params[:maxIterations] = 100
			
			#begin
			res = nil
			realtime = Benchmark.realtime do
				res = sm.scan_matching
			end
			
				
				x = res[:x];
				e_xy = sqrt(x[0]*x[0]+x[1]*x[1]);
				e_th = x[2].abs
				e_max = [e_xy,e_th].max
				hist.increment2(e_max);
				hist_iterations.increment2(res[:iterations])
				
				bin = hist.find(e_max);
				codes[bin].push code

				puts ">>> #{sm.name} #{code} #{bin} >>> x = #{pv(res[:x])} it = #{res[:iterations]} "+
				"time =#{two_decimals(realtime)} "+
					"error #{res[:error]}  disp #{pv(disp)}"
			#rescue
			#	puts "Failed #{code}"
			#	failed_codes.push code
			#	failed += 1;
			#end
		#	exit if ARGV.include?(code)
		end end
		
		f.puts "\n\n==== Experiment #{i+1}: #{pv(max_displacement[i])}"
		f.puts " (software errors: #{failed}/#{scans.size*repetitions_per_scan})"
		f.puts " Errors "
		f.puts "------------------"
		write_summary(f, hist);
		f.puts " Iterations"
		f.puts "------------------"
		write_summary(f, hist_iterations);
		f.puts " Experiments with large errors: "
		for i in 2..4
			f.puts "  bin #{i}: #{codes[i].join(' ')} " 
		end
		f.flush
	end
		
	f.puts "\n\nFailed experiments: #{failed_codes.join(', ')}"
end

scan_matcher = eval ARGV.shift

log = 'laserazosSM3.off'
log = 'a.off'
scans = nil
File.open(log) do |f| 
	scans = read_log(f) 
end

main(scans, scan_matcher)













