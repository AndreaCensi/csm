
require 'rsm'
require 'rsm_icp_corr_dumb'
require 'rsm_icp_corr_tricks'
require 'rsm_icp_cov_numeric'
require 'rsm_icp_cov_exact'
require 'lib/gpc.rb'

class ICP
	include GSL
	include MathUtils	
	include Journal
	include Math
	
	attr_accessor :params
	
	def initialize
		@params = standard_parameters
	end

	
	class Correspondence
		attr_accessor :i
		attr_accessor :j1
		attr_accessor :j2
	end
	
	def scan_matching
		
		journal_laser 'laser_ref', params[:laser_ref]
		journal_laser 'laser_sens', params[:laser_sens]
		journal "odometry #{to_j(params[:firstGuess])}"
		journal "odometry_cov #{to_j(params[:firstGuessCov])}"

		
		x_old = params[:firstGuess]
		delta = Vector.alloc(0,0,0).col
		
		for iteration in 1..params[:maxIterations]
			journal "iteration #{iteration}"
			journal     "x_old #{to_j(x_old)}"

			corrs = find_correspondences_tricks(x_old)
			#corrs = find_correspondences(x_old)
			
			x_new, dx_dy1, dx_dy2 = compute_next_estimate(corrs, x_old)

			sigma = 0.01 # cm
			
			cov_x = (sigma*sigma) * dx_dy1 * (dx_dy1.trans) +
			        (sigma*sigma) * dx_dy2 * (dx_dy2.trans);
			

			journal     "x_new #{to_j(x_new)}"

			new_delta = pose_diff(x_new, x_old)
			journal     "delta #{to_j(new_delta)}"
			
	#		puts "#{iteration} x_new = #{pv(x_new)} delta = #{pv(new_delta)}"+
	#			" neg #{new_delta.trans * delta>0} error #{@total_error} "
	#		puts "    cov_x #{pm(cov_x)}"
			delta = new_delta
		
			if delta[0,1].nrm2 < params[:epsilon_xy] &&
			   delta[2].abs < params[:epsilon_theta]
				
				break
			end
			
			x_old = x_new
		end
		
		res = Hash.new
		res[:x] = x_new
		res[:iterations] = iteration
		res[:dx_dy1] = dx_dy1
		res[:dx_dy2] = dx_dy2
		
		return res
	end
	
	
	require 'point2line'

	# f should be a Proc
	def deriv(f, x,eps)
		(f.call(x+eps)-f.call(x-eps))/(2*eps)
	end
	
	def compute_next_estimate(correspondences, x_old)
		
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		## current distances
		dists = Array.new
		@total_error = 0
		corrs = Array.new
		for c in correspondences; next if c.nil?
			p_i  = laser_sens.points[c.i ].cartesian
			p_j1 = laser_ref .points[c.j1].cartesian
			p_j2 = laser_ref .points[c.j2].cartesian
			
			c2 = PointCorrespondence.new
			c2.p = p_i
			c2.q = p_j1
			v_alpha = rot(PI/2) * (p_j1-p_j2)
			v_alpha = v_alpha / v_alpha.nrm2
#			puts "#{c.i} #{c.j1} #{c.j2} #{v_alpha}"
			c2.C = v_alpha*v_alpha.trans
		
			# Point to point
			#	c2.C =Matrix.eye(2)
			# mixed
		#	c2.C = 0.5 * Matrix.eye(2) + 0.5 *(v_alpha*v_alpha.trans)	
		
			corrs[c.i] = c2

			dists[c.i] =
				(transform(c2.p, x_old)-c2.q).trans * c2.C *
				(transform(c2.p, x_old)-c2.q)
			@total_error += dists[c.i]
	#		dists[c.i] = sqrt(dists[c.i])
		end

#	journal_correspondences("candidate", correspondences)
#	kill_outliers(correspondences, corrs, dists)
	journal_correspondences("correspondences", correspondences)
	
	#max = dists.compact.max
	#	puts "Dists: #{dists.compact.map{|x| (x*1000).ceil}.join(',')}"
	#	puts "Dists: #{dists.map{|x| x.nil? ?nil : x / max }.
	#		map{|x| x.nil? ? nil : (x*1000).ceil/1000}.join(',')}"
		x_new = general_minimization(corrs.compact)
		
#		cov = compute_covariance(laser_ref, laser_sens, correspondences, x_old)
		dx_dy1, dx_dy2 = 
			compute_covariance_exact(laser_ref, laser_sens, correspondences, x_old)
		
		
		return x_new, dx_dy1, dx_dy2
	end

	def kill_outliers(correspondences, corrs, dists)
		max_bin = 0.5;
		bins = 100
		perc = 0.4
		safe_level = 5

		hist = GSL::Histogram.alloc(bins, 0, max_bin)
		dists.each do |d| hist.fill2 d unless d.nil? end
		int = hist.integrate.scale(1.0/dists.compact.size)
		# XXX sono 
		#puts "Histo:"

#		for i in 0..bins-1
#			$stdout.write " #{hist[i]} "
#		end

		for i in 0..bins-1
			if int[i] > perc
				superato = i
				break
			end
		end

		nkilled = 0
		dists.each_index do |i|
			next if corrs[i].nil?
			bin = (dists[i]/max_bin)*bins;
	#		puts "#{i} bin=#{bin} dist=#{dists[i]}"
			if bin > (superato+1) * safe_level
#				puts "kill #{i} (d=#{dists[i]})"
				corrs[i] = nil;
				correspondences[i] = nil;
				nkilled += 1
			end
		end
		
		puts "killed #{nkilled}/#{corrs.size}"
	end

end
