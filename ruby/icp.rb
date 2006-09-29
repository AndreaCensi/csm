require 'gsl'
require 'structures'
require 'icp_correspondences_exact'
require 'icp_correspondences_tricks'
require 'journal'



class ICP
	include GSL
	include MathUtils	
	include Journal
	include Math
	
	attr_accessor :params
	
	def initialize
		@params = Hash.new
		standard_parameters
	end

	def standard_parameters
		@params[:maxAngularCorrectionDeg]= 30
		@params[:maxLinearCorrection]=    0.5
		@params[:maxCorrespondenceDist]=   2
		@params[:maxIterations]=           40
		@params[:firstGuess]=         Vector.alloc(0,0,0)
		@params[:interactive]=  false
		@params[:epsilon_xy]=  0.001
		@params[:epsilon_theta]=   MathUtils.deg2rad(0.01)
		@params[:sigma]=           0.01
		@params[:do_covariance]=  false
	end
	
	
	class Correspondence
		attr_accessor :i
		attr_accessor :j1
		attr_accessor :j2
	end
	
	def scan_matching

		journal_laser 'laser_ref', params[:laser_ref]
		journal_laser 'laser_sens', params[:laser_ref]
		journal "odometry #{to_j(params[:firstGuess])}"
		journal "odometry_cov #{to_j(params[:firstGuessCov])}"

		@p_j = Array.new
		for j in 0..params[:laser_ref].nrays-1
			if params[:laser_ref].points[j].valid? 
				@p_j[j] = params[:laser_ref].points[j].cartesian
			end
		end
		
		x_old = params[:firstGuess]
		delta = Vector.alloc(0,0,0).col
		
		for iteration in 1..params[:maxIterations]
			journal "iteration #{iteration}"
			journal     "x_old #{to_j(x_old)}"

			corrs = find_correspondences_tricks(x_old)
			journal_correspondences corrs
			
			x_new = compute_next_estimate(corrs, x_old)
			journal     "x_new #{to_j(x_new)}"

			new_delta = pose_diff(x_new, x_old)
			journal     "delta #{to_j(new_delta)}"
			
			puts "#{iteration} x_new = #{pv(x_new)} delta = #{pv(new_delta)}"+
				" neg #{new_delta.trans * delta>0} error #{@total_error} "
			delta = new_delta
		
			if delta[0,1].nrm2 < params[:epsilon_xy] &&
			   delta[2].abs < params[:epsilon_theta]
				
				break
			end
			
			x_old = x_new
		end
		
	end
	
	
	require 'point2line'

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
			#			c2.C = c2.C  + 0.01 *Matrix.eye(2)
#			c2.C =Matrix.eye(2)
			corrs.push c2

			dists[c.i] =
				(transform(c2.p, x_old)-c2.q).trans * c2.C *
				(transform(c2.p, x_old)-c2.q)
			@total_error += dists[c.i]
			#dists[c.i] = sqrt(dists[c.i])
		end

		#max = dists.compact.max
	#	puts "Dists: #{dists.compact.map{|x| (x*1000).ceil}.join(',')}"
	#	puts "Dists: #{dists.map{|x| x.nil? ?nil : x / max }.
	#		map{|x| x.nil? ? nil : (x*1000).ceil/1000}.join(',')}"
		return general_minimization(corrs)
	end
	
	
end
