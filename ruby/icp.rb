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
		journal_laser 'laser_sens', params[:laser_sens]
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
			#corrs = find_correspondences(x_old)
			
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

#	kill_outliers(correspondences, corr, dists)
	journal_correspondences correspondences

	
	#max = dists.compact.max
	#	puts "Dists: #{dists.compact.map{|x| (x*1000).ceil}.join(',')}"
	#	puts "Dists: #{dists.map{|x| x.nil? ?nil : x / max }.
	#		map{|x| x.nil? ? nil : (x*1000).ceil/1000}.join(',')}"
		x_new = general_minimization(corrs.compact)
		
		cov = compute_covariance(laser_ref, laser_sens, correspondences, x_old)
		
		x_new
	end

	def kill_outliers(correspondences, corr, dists)
		max_bin = 0.5;
		bins = 100
		perc = 0.4
		safe_level = 3

		hist = GSL::Histogram.alloc(bins, 0, max_bin)
		dists.each do |d| hist.fill2 d unless d.nil? end
		int = hist.integrate.scale(1.0/dists.compact.size)
		# XXX sono 
		puts "Histo:"

		for i in 0..bins-1
			$stdout.write " #{hist[i]} "
		end

		for i in 0..bins-1
			if int[i] > perc
				superato = i
				break
			end
		end

		dists.each_index do |i|
			next if corrs[i].nil?
			bin = (dists[i]/max_bin)*bins;
	#		puts "#{i} bin=#{bin} dist=#{dists[i]}"
			if bin > (superato+1) * safe_level
				puts "kill #{i} (d=#{dists[i]})"
				corrs[i] = nil;
				correspondences[i] = nil;
			end
		end
	end

	def compute_covariance(laser_ref, laser_sens, correspondences, x_new)
		fJ = create_J_function(laser_ref, laser_sens, correspondences)

		x  = x_new
		y1 =  laser_ref.points.map{|p| p.reading}
		y2 = laser_sens.points.map{|p| p.reading}

		tot = fJ.call(x,y1,y2)
		 eps_x =0.00001; eps_th = deg2rad(0.00001)
		puts "tot= #{tot} , total_error = #{@total_error}"
		d0 = Vector[eps_x,0, 0].col;
		d1 = Vector[0,eps_x, 0].col;
		d2 = Vector[0,0,eps_th].col;
		
		mm = Matrix.alloc(3,3); 
		mm[0,0]=3.0;
		mm[1,1]=5.0;
		mm[2,2]=7.0;
		puts "m = \n #{mm}"
		fJ = Proc.new {|x,y1,y2|
			0.5 *(x.trans * mm * x)
		}
		dJ_dx = Proc.new { |x,y1,y2|
			dJ_dx0 = (fJ.call(x,y1,y2)-fJ.call(x-d0,y1,y2))/(eps_x)
			dJ_dx1 = (fJ.call(x,y1,y2)-fJ.call(x-d1,y1,y2))/(eps_x)
			dJ_dx2 = (fJ.call(x,y1,y2)-fJ.call(x-d2,y1,y2))/(eps_th)
			Vector.alloc(dJ_dx0,dJ_dx1,dJ_dx2).col
		}
		d2J_dx2 = Proc.new { |x,y1,y2|
			d2J_dx0 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d0,y1,y2))/(eps_x);
			d2J_dx1 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d1,y1,y2))/(eps_x);
			d2J_dx2 = (dJ_dx.call(x,y1,y2)-dJ_dx.call(x-d2,y1,y2))/(eps_th);
			m = Matrix.alloc(3,3)
			m.set_col(0, d2J_dx0)
			m.set_col(1, d2J_dx1)
			m.set_col(2, d2J_dx2)
			m
		}
		
		dJ_dxee=  dJ_dx.call(x,y1,y2)
		puts "deriv= #{dJ_dxee}"

		d2 = d2J_dx2.call(x,y1,y2)
		puts "d2J_dx2 =\n #{d2}"
	end
	
	def create_J_function(laser_ref, laser_sens, correspondences)
		Proc.new { |x,y1,y2|
			fJtot = 0;
			for c in correspondences
				next if c.nil? 
				p_i  = laser_sens.points[c.i ].v * y2[c.i]
				p_j1 = laser_ref .points[c.j1].v * y1[c.j1]
				p_j2 = laser_ref .points[c.j2].v * y1[c.j2]

				v_alpha = rot(PI/2) * (p_j1-p_j2)
				v_alpha = v_alpha / v_alpha.nrm2
				m = v_alpha*v_alpha.trans

				p_i_w = transform(p_i, x)
				fJtot +=	(p_i_w-p_j1).trans * m * (p_i_w-p_j1)
			end
			fJtot 
		}
	end
	
end
