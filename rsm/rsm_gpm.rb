require 'rsm'
require 'rsm_clustering'
require 'rsm_orientation'

class GPM
	include MathUtils
	include Math
	include Journal
	attr_accessor :params

	def name; "GPM" end
	
	def scan_matching 
		laser_ref = params[:laser_ref]
		laser_sens = params[:laser_sens]
		u = params[:firstGuess]
		
		laser_ref.compute_cartesian
		laser_ref.simple_clustering(params[:clustering_threshold])
		laser_ref.compute_orientation(params[:orientation_neighbourhood],params[:sigma])

		laser_sens.compute_cartesian		
		laser_sens.simple_clustering(params[:clustering_threshold])
		laser_sens.compute_orientation(params[:orientation_neighbourhood],params[:sigma])

		journal_laser 'laser_ref', laser_ref
		journal_laser 'laser_sens',laser_sens
		journal "odometry #{to_j(u)}"
#		journal "odometry_cov #{to_j(params[:firstGuessCov])}"
		
		max_angular_correction_deg = params[:max_angular_correction_deg]
		max_linear_correction =params[:max_linear_correction]
		
		theta_bin_size = deg2rad(5)
		extend_range = deg2rad(15)
		# Find multiple solutions for theta
		hist = GSL::Histogram.alloc((2*PI/theta_bin_size).ceil, -PI, PI)
		
		ght_only_theta(u, max_linear_correction, max_angular_correction_deg, hist)
		
		hist.fprintf($stdout)
		# find mode 
		max_bin = hist.max_bin
		new_range = hist.get_range(max_bin)
		new_range[0] += -extend_range
		new_range[1] += +extend_range
		puts "New extension: #{rad2deg(new_range[0])}° to #{rad2deg(new_range[1])}°"
		u2 = u.clone; u2[2] = 0.5*(new_range[1]+new_range[0]);
		
		newAngularCorrectionDeg =  rad2deg(0.5*(new_range[1]-new_range[0]))
		matches2 = ght(u2, max_linear_correction,newAngularCorrectionDeg)
		puts "Found #{matches2.size} matches."
		result = solve_lse(matches2, u2, 6)
		puts "Result #{pv(result)}."

		
		
		journal "iteration 0"
		journal     "x_old #{to_j(u)}"
		journal "iteration 1"
		journal     "x_old #{to_j(u2)}"
		journal "iteration 2"
		journal     "x_old #{to_j(result)}"
		
		
		res = Hash.new
		res[:x] = result
		res[:iterations] = 0
		#res[:dx_dy1] = dx_dy1
		#res[:dx_dy2] = dx_dy2
		
		return res
	end	
	
	def solve_lse(matches, x_old, iterations)		
		iterations.times do 
			z = Matrix.alloc(3,1); z.set_all(0);
			w = Matrix.alloc(3,3); w.set_all(0);
			for m in matches
				v_alpha = vers(m.alpha)
				# Y = L x
				y = Matrix[ [v_alpha.trans * m.t],
								[m.theta]];
				l = Matrix[ 
					[ v_alpha[0], v_alpha[1], 0],
					[0, 0, 1]];
				weight = m.weight * exp(-(m.t-x_old[0,1]).nrm2**2 - 5 * (m.theta-x_old[2]).abs**2)
				z = z + weight * l.trans * y;
				w = w + weight * l.trans * l;
			end
			x = w.inv * z
			x_old = Vector.alloc(x[0,0],x[1,0],x[2,0])
			puts "Iteration: #{pv(x_old)}"
		end
		x_old
	end
	
	
	
	
	class Match
		attr_accessor :t
		attr_accessor :theta
		attr_accessor :alpha
		attr_accessor :weight
	end
	
	
	def ght(x0,max_linear_correction,max_angular_correction_deg)
		puts "ght: x0=#{x0}, limits: #{max_linear_correction}, #{max_angular_correction_deg}°"
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];
		
		t0 = Vector[x0[0],x0[1]]
		matches = Array.new
		for i in 0..laser_sens.nrays-1
			next if not laser_sens.alpha_valid? i

			p_i = laser_sens.p[i]
			alpha_i = laser_sens.alpha[i]

			from, to = possible_interval(p_i, laser_ref, 
				max_angular_correction_deg, max_linear_correction)
			
			for j in from..to
				next if not laser_ref.alpha_valid? j
				
				p_j = laser_ref.p[j]
				alpha_j = laser_ref.alpha[j]

				theta = angleDiff(alpha_j, alpha_i)
				
				next if (theta-x0[2]).abs > deg2rad(max_angular_correction_deg)
				
				t = p_j - rot(theta) * p_i;
				
				next if (t-t0).nrm2 > max_linear_correction
				
				m = Match.new
				m.t = t; 
				m.theta=theta;
				m.alpha = alpha_j
				w = laser_ref.cov_alpha[j] + laser_sens.cov_alpha[i]
				m.weight = 1/w
				matches.push m
			end 
			
		end 

		matches
	end

	def ght_only_theta(x0,max_linear_correction,max_angular_correction_deg,hist)
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		t0 = Vector[x0[0],x0[1]]
		for i in 0..laser_sens.nrays-1
			next if not laser_sens.alpha_valid? i

			p_i = laser_sens.p[i]
			alpha_i = laser_sens.alpha[i]

			from, to = possible_interval(p_i, laser_ref, 
				max_angular_correction_deg, max_linear_correction)
			
			for j in from..to
				next if not laser_ref.alpha_valid? j
				
				p_j = laser_ref.p[j]
				alpha_j = laser_ref.alpha[j]

				theta = angleDiff(alpha_j, alpha_i)
				
				next if (theta-x0[2]).abs > deg2rad(max_angular_correction_deg)			
								
				t = p_j - rot(theta) * p_i;
				
				next if (t-t0).nrm2 > max_linear_correction
				
				hist.increment(theta)
			end 
			
		end 
	end

end








