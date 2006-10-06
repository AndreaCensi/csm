require 'icpc_wrap'
require 'icp'


class GPM_then_ICP
	attr_accessor :params
	
	def initialize
		@params = Hash.new
		@journal = nil
	end
	
	def name
		'GPM+ICP'
	end
	
	def journal_open(file)
		@journal = file
	end
	
	def scan_matching
		gpm = GPMC.new
		gpm.params = params
		if not @journal.nil?
			gpm.journal_open(@journal+"_gpm")
		end
		
		gpm_x = (gpm.scan_matching)[:x]
			
		puts "GPM_then_ICP: gpm res is #{pv(gpm_x)}"
		icpc = ICPC.new
		icpc.params = params;
		icpc.params[:firstGuess] = gpm_x

		if not @journal.nil?
			puts "opening"
			icpc.journal_open(@journal+"_icp")
		end
		
		res = icpc.scan_matching;
=begin		
		pt = 0.01; pth = deg2rad(1.5);
		perturb = [
			Vector[+pt,0,0],
			Vector[-pt,0,0],
			Vector[0,+pt,0],
			Vector[0,-pt,0],
			Vector[0,0,+pth],
			Vector[0,0,-pth],
		];
		
		params[:maxLinearCorrection] = pt*5;
		params[:maxAngularCorrectionDeg] = pth*5;
		
		total_iterations = res[:iterations]
		results = perturb.map { |p| 
			icpc.params[:firstGuess] = res[:x] + p;
			puts "icp, restarting with perturb " + pv(p)
			r = icpc.scan_matching
			total_iterations += r[:iterations]
			r
		}
		results.push res
		results.sort!{|x,y| x[:error]<=> y[:error]}

		
 		puts "Choose:"
 		puts "  -- x0 = #{pv(res[:x])}, error = #{res[:error]}"

		results.each do |r|
	 		puts "  -- x = #{pv(r[:x])}, error = #{r[:error]}"
		end
		
		results[0][:iterations] = total_iterations
		results[0]
=end		
		res
	end
end