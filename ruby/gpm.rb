require 'journal'
require 'mathutils'

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
		
		
		clustering(laser_ref)
		compute_orientation(laser_ref)
		clustering(laser_sens)
		compute_orientation(laser_sens)

		journal_laser 'laser_ref', laser_ref
		journal_laser 'laser_sens',laser_sens
		journal "odometry #{to_j(u)}"
#		journal "odometry_cov #{to_j(params[:firstGuessCov])}"
		
		maxAngularCorrectionDeg = params[:maxAngularCorrectionDeg]
		maxLinearCorrection =params[:maxLinearCorrection]
		
		theta_bin_size = deg2rad(5)
		extend_range = deg2rad(15)
		# Find multiple solutions for theta
		hist = GSL::Histogram.alloc((2*PI/theta_bin_size).ceil, -PI, PI)
		
		ght_only_theta(u, maxLinearCorrection, maxAngularCorrectionDeg, hist)
		
		hist.fprintf($stdout)
		# find mode 
		max_bin = hist.max_bin
		new_range = hist.get_range(max_bin)
		new_range[0] += -extend_range
		new_range[1] += +extend_range
		puts "New extension: #{rad2deg(new_range[0])}° to #{rad2deg(new_range[1])}°"
		u2 = u.clone; u2[2] = 0.5*(new_range[1]+new_range[0]);
		
		newAngularCorrectionDeg =  rad2deg(0.5*(new_range[1]-new_range[0]))
		matches2 = ght(u2, maxLinearCorrection,newAngularCorrectionDeg)
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
	
	def clustering(ld)
		sigma = 0.01;
		count = 0;
		last_reading = nil;
		ld.points.each_index do |i|;
			if not ld.points[i].valid?
				ld.points[i].cluster = -1
				next
			end
			if last_reading.nil?
				ld.points[i].cluster = count;
			else
				reading = ld.points[i].reading
				if(last_reading-reading).abs > 5*sigma
					count += 1
					ld.points[i].cluster = count;
				else
					ld.points[i].cluster = count;
				end
			end
			last_reading = ld.points[i].reading
		end
	end

	# uses params[:gpm_neighbours]?
	def find_neighbours(ld, i)
		num = 3;
		up = i; 
		while (up+1 <= i+3) and (up+1<ld.nrays) and (ld.points[up+1].valid?) and 
				(ld.points[up+1].cluster == ld.points[i].cluster)
			up+=1; 
		end
		down = i; 
		while (down >= i-3) and (down-1>=0) and (ld.points[down-1].valid?) and 
				(ld.points[down-1].cluster == ld.points[i].cluster)
			down-=1;
		end
		(down..(i-1)).to_a + ((i+1)..up).to_a
	end
		
	# Computes alpha for each point
	def compute_orientation(ld)
		ld.points.each_index do |i|; 
			if not ld.points[i].valid?
				ld.points[i].alpha = nil
				ld.points[i].alpha_valid = false
				next
			end
			
			# list of index
			neighbours = find_neighbours(ld, i)
			
#			puts "i = #{i} neigbours = #{neighbours.join(', ')}"

			if neighbours.size == 0
				ld.points[i].alpha = nil
				ld.points[i].alpha_valid = false
				next
			end
			

			thetas = neighbours.map{|j| ld.points[j].theta}
			readings = neighbours.map{|j| ld.points[j].reading}
			theta   = ld.points[i].theta
			reading = ld.points[i].reading
			alpha, cov0_alpha = filter_orientation(theta,reading,thetas,readings,1);
			cov0_alpha *= params[:sigma]*params[:sigma]
		#	puts "cov = #{rad2deg(sqrt(cov0_alpha))} deg"
			ld.points[i].alpha = alpha
			ld.points[i].cov_alpha = cov0_alpha 
			ld.points[i].alpha_valid = alpha.nan? ? false : true
			if alpha.nan?
				puts "We have a problem.."
			end
		end
	end
	
	def filter_orientation(theta0, rho0, thetas, rhos, curv)
		n = thetas.size
		# y = l x + r epsilon
		y = Matrix.alloc(n, 1)
		l = Matrix.alloc(n, 1)
		r = Matrix.alloc(n, n+1)
		
		r.set_all(0.0)
		l.set_all(1.0);
		
		for i in 0..n-1
		#	puts "theta0 = #{theta0}  thetas[i] = #{thetas[i]}"
			y[i,0]   = (rhos[i]-rho0)/(thetas[i]-theta0)
			r[i,0]   = -1/(thetas[i]-theta0);
			r[i,i+1] =  1/(thetas[i]-theta0);
		end
		
		#puts "y = \n#{y}"
		#puts "l = \n#{l}"
		#puts "r = \n#{r}"
		
		# x = (l^t R^-1 l)^-1 l^t R^-1 y
		f1 = ((l.trans * (r*r.trans).inv * l).inv * l.trans * (r*r.trans).inv * y)[0,0]
		
		#alpha = theta0 + PI/2 + Math.atan(rho0/f1)
		alpha = theta0 + Math.atan(f1/rho0)
		
		cov_f1 = ((l.trans * l).inv)[0,0]
		
		dalpha_df1  = rho0 / (rho0**2 + f1**2)
		dalpha_drho = -f1 / (rho0**2 + f1**2)
		cov0_alpha = (dalpha_df1**2) * cov_f1 + (dalpha_drho**2)

		#puts " cov_f1 = #{cov_f1} dalpha_df1 #{dalpha_df1**2} dalpha_drho #{dalpha_drho**2} "
		return alpha, cov_f1
	end
	
	
	
	class Match
		attr_accessor :t
		attr_accessor :theta
		attr_accessor :alpha
		attr_accessor :weight
	end
	
	def angleDiff(a,b)
		d = a - b
		
		while d < -PI; d+=2*PI; end
		while d >  PI; d-=2*PI; end
		d
	end
	
	def ght(x0,maxLinearCorrection,maxAngularCorrectionDeg)
		puts "ght: x0=#{x0}, limits: #{maxLinearCorrection}, #{maxAngularCorrectionDeg}°"
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		p_j = laser_ref.points.map{ |p| p.valid? ? p.cartesian : nil}
		
		t0 = Vector[x0[0],x0[1]]
		matches = Array.new
		for i in 0..laser_sens.nrays-1
			next if not laser_sens.points[i].alpha_valid?

			p_i = laser_sens.points[i].cartesian

			from, to = possible_interval(p_i, laser_sens, 
				maxAngularCorrectionDeg, maxLinearCorrection)
			
			for j in from..to
				next if not laser_ref.points[j].alpha_valid?
				
				theta = angleDiff(laser_ref.points[j].alpha, laser_sens.points[i].alpha)
				
				next if (theta-x0[2]).abs > deg2rad(maxAngularCorrectionDeg)
								
				t = p_j[j] - rot(theta) * p_i;
				
				next if (t-t0).nrm2 > maxLinearCorrection
				
				m = Match.new
				m.t = t; m.theta=theta;
				m.alpha = laser_ref.points[j].alpha
				w = laser_ref.points[j].cov_alpha + laser_sens.points[i].cov_alpha
				m.weight = 1/w
				matches.push m
			end 
			
		end 

		matches
	end

	def ght_only_theta(x0,maxLinearCorrection,maxAngularCorrectionDeg,hist)
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		p_j = laser_ref.points.map{ |p| p.valid? ? p.cartesian : nil}
		t0 = Vector[x0[0],x0[1]]
		for i in 0..laser_sens.nrays-1
			next if not laser_sens.points[i].alpha_valid?

			p_i = laser_sens.points[i].cartesian

			from, to = possible_interval(p_i, laser_sens, 
				maxAngularCorrectionDeg, maxLinearCorrection)
			
			for j in from..to
				next if not laser_ref.points[j].alpha_valid?
				
				theta = angleDiff(laser_ref.points[j].alpha, laser_sens.points[i].alpha )
				
				if theta.nan?
					puts "BUGGG"
					puts "j = #{j} alpha_j = #{laser_ref.points[j].alpha}"
					puts "i = #{i} alpha_i = #{laser_sens.points[i].alpha}"
					next
				end

				next if (theta-x0[2]).abs > deg2rad(maxAngularCorrectionDeg)			
								
				t = p_j[j] - rot(theta) * p_i;
				
				next if (t-t0).nrm2 > maxLinearCorrection
				
				hist.increment(theta)
			end 
			
		end 
	end

end








