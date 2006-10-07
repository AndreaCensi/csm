
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

	def name; "ICP" end
		
	def scan_matching
		laser_ref = params[:laser_ref]
		laser_sens = params[:laser_sens]
		
		laser_ref.compute_cartesian
		laser_ref.simple_clustering(params[:clusteringThreshold])
		laser_ref.compute_orientation(params[:orientationNeighbourhood],params[:sigma])

		laser_sens.compute_cartesian		
		laser_sens.simple_clustering(params[:clusteringThreshold])
		laser_sens.compute_orientation(params[:orientationNeighbourhood],params[:sigma])
		
		laser_ref.create_jump_tables
		
		journal_laser 'laser_ref', laser_ref 
		journal_laser 'laser_sens', laser_sens 
		journal "odometry #{to_j(params[:firstGuess])}"
		journal "odometry_cov #{to_j(params[:firstGuessCov])}"

		x_old = params[:firstGuess]
		delta = Vector.alloc(0,0,0).col
		
		for iteration in 1..params[:maxIterations]
			journal "iteration #{iteration}"
			journal     "x_old #{to_j(x_old)}"

			find_correspondences_tricks(x_old)
			
			x_new, dx_dy1, dx_dy2 = compute_next_estimate( x_old)

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
	
	
	# f should be a Proc
#	def deriv(f, x,eps)
#		(f.call(x+eps)-f.call(x-eps))/(2*eps)
	#end
	
	def compute_next_estimate(x_old)
		
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		## current distances
		dists = []
		@total_error = 0
		corrs = []
		for i in 0..laser_sens.nrays-1
			next if not laser_sens.corr[i].valid
			p_i  = laser_sens.p[i ]
			p_j1 = laser_ref.p[laser_sens.corr[i].j1]
			p_j2 = laser_ref.p[laser_sens.corr[i].j2]
			
			c2 = GPC::PointCorrespondence.new
			c2.p = p_i
			c2.q = p_j1
			v_alpha = rot(PI/2) * (p_j1-p_j2)
			v_alpha = v_alpha / v_alpha.nrm2
			c2.C = v_alpha*v_alpha.trans
		
			corrs[i] = c2

			dists[i] =
				(transform(c2.p, x_old)-c2.q).trans * c2.C *
				(transform(c2.p, x_old)-c2.q)
			@total_error += dists[i]
	#		dists[c.i] = sqrt(dists[c.i])
		end

#	journal_correspondences("candidate", correspondences)
	journal_correspondences("correspondences", laser_sens.corr)
	

	corrs = corrs.compact
	puts "Found #{corrs.size} correspondences."
		x_new = GPC.gpc(corrs)
		
#		cov = compute_covariance(laser_ref, laser_sens, correspondences, x_old)
#		dx_dy1, dx_dy2 = 
#			compute_covariance_exact(laser_ref, laser_sens,  x_old)
		
		dx_dy1=Matrix.alloc(3,laser_ref.nrays)
		dx_dy2=Matrix.alloc(3,laser_sens.nrays)
		return x_new, dx_dy1, dx_dy2
	end

end
