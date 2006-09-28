require 'gsl'
require 'structures'



class ICP < Hash
	include GSL
	include MathUtils
	include Math
	
	attr_accessor :params
	
	def initialize
	
		@correspondences = Array.new
	
		@params = Hash.new
		standard_parameters
	end

	def standard_parameters
		param :maxAngularCorrectionDeg, 30
		param :maxLinearCorrection,     0.5
		param :maxCorrespondenceDist,   2
		param :maxIterations,           40
		param :firstGuess,         Vector.alloc(0,0,0)
		param :interactive,  false
		param :epsilon_xy,  0.001
		param :epsilon_theta,   MathUtils.deg2rad(0.01)
		param :sigma,           0.01
		param :do_covariance,  false
	end
	
	def param(symbol, value)
		@params[symbol] = value
	end
end

class ICP
	class Correspondence
		attr_accessor :i
		attr_accessor :j1
		attr_accessor :j2
	end
	
	def scan_matching
		
		@p_j = Array.new
		for j in 0..params[:laser_ref].nrays-1
			if params[:laser_ref].points[j].valid? 
				@p_j[j] = params[:laser_ref].points[j].cartesian
			end
		end
		
		x_old = params[:firstGuess]
		puts "0 x_old = #{pv(x_old)}"
		
		delta = Vector.alloc(0,0,0).col
		for iteration in 1..params[:maxIterations]
#			puts "Iteration #{iteration}"
		
			find_correspondences(x_old)
		
			x_new = compute_next_estimate(x_old)

			new_delta = pose_diff(x_new, x_old)
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
	
	def ICP.possible_interval(point, ld, maxAngularCorrectionDeg, 
		maxLinearCorrection)
		
		angleRes =  (ld.max_theta-ld.min_theta)/ld.nrays;

		# Delta for the angle
		delta = deg2rad(maxAngularCorrectionDeg).abs +
		        Math.atan(maxLinearCorrection/point.nrm2).abs;

		# Dimension of the cell range
		range = (delta/angleRes).ceil;
		
		# To be turned into an interval of cells
		start_theta = Math.atan2(point[1],point[0]);
		
		start_cell  = 
			(start_theta - ld.min_theta) / (ld.max_theta-ld.min_theta) * ld.nrays;
	
		start_cell = start_cell.ceil
		
		# Pay attention to minimum and maximum
		from = min(ld.nrays-1, max( (start_cell-range).floor,0));
		to = max(0, min((start_cell+range).ceil,ld.nrays-1));
	
		if false
			puts "start_theta=#{rad2deg(start_theta)}°, "+
			     "range = #{range} cells,"+
			     "start_cell=#{rad2deg(start_cell)},"+
				  "fromto=#{from}->#{to}"
		end
		
		return from, to, start_cell, range
	end
	
	DESCRIBE = false
	VERIFY_EXACT = false
	JUMP_TRICK = true
	JUMP_TRICK_DELTA = 0
		
	def find_correspondences(x_old);
		maxAngularCorrectionDeg = params[:maxAngularCorrectionDeg]
		maxLinearCorrection=params[:maxLinearCorrection]
		maxDist = params[:maxCorrespondenceDist]
		
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];
		ld = laser_sens
		last_diff = 0
		
		dbg_num_a1 = 0
		last_best = nil

		
		if JUMP_TRICK
			@up_bigger = Array.new
			@up_smaller = Array.new
			@down_bigger = Array.new
			@down_smaller = Array.new
			readings = (0..laser_ref.nrays-1).map{ |j| 
				laser_ref.points[j].reading}
			delta = 0;
			readings.each_index{ |i|
				j = i + 1;
				while (not readings[j].nil?) and 
					readings[j]<=readings[i]+JUMP_TRICK_DELTA
					j += 1;
				end
				@up_bigger[i] = j-i;

				j = i + 1;
				while (not readings[j].nil?) and 
					readings[j]+JUMP_TRICK_DELTA>=readings[i]
					j += 1;
				end
				@up_smaller[i] = j-i;

				j = i - 1;
				while (not readings[j].nil?) and 
					readings[j]+JUMP_TRICK_DELTA>=readings[i]
					j -= 1;
				end
				@down_smaller[i] = j-i;
				
				j = i - 1;
				while (not readings[j].nil?) and 
					readings[j]<=readings[i]+JUMP_TRICK_DELTA
					j -= 1;
				end
				@down_bigger[i] = j-i;
				
			}
		end
#		puts "Bigger:  #{@down_bigger. join(',')}"
#		puts "Smaller: #{@down_smaller.join(',')}"

		for i in 0..laser_sens.nrays-1
			if not laser_sens.points[i].valid?
				@correspondences[i] = nil
				next
			end
			
			p_i = laser_sens.points[i].cartesian
			p_i_w = transform(p_i, x_old);
			p_i_w_nrm2 = p_i_w.nrm2
	
			angleRes =  (ld.max_theta-ld.min_theta)/ld.nrays;
	
			# Delta for the angle
			delta = deg2rad(maxAngularCorrectionDeg).abs +
					  Math.atan(maxLinearCorrection/p_i_w_nrm2).abs;
	
			# Dimension of the cell range
			range = (delta/angleRes).ceil;
			
			# To be turned into an interval of cells
			start_theta = Math.atan2(p_i_w[1],p_i_w[0]);
			
			start_cell=((start_theta-ld.min_theta)/(ld.max_theta-ld.min_theta)*
				ld.nrays).ceil
			
			from = [ld.nrays-1, [(start_cell-range).floor,0].max ].min
			to =  [0, [(start_cell+range).ceil,ld.nrays-1].min ].max
		
			## Find best correspondence with exact algorithm
			best_j = nil;
			if VERIFY_EXACT
				best_j_dist = 0;
				$stderr.write "#{i}: " if DESCRIBE
				for j in from..to
					$stderr.write "#{j}" if DESCRIBE
					if @p_j[j].nil?
						$stderr.write "N" if DESCRIBE
						next
					end
					## Find compatible interval in the other scan. 
					dist = (p_i_w - @p_j[j]).nrm2
					if dist > maxDist
						$stderr.write "M" if DESCRIBE
						next
					end
					if (best_j.nil?) || (dist < best_j_dist)
						$stderr.write "*" if DESCRIBE
						best_j = j; best_j_dist = dist;
					end
				end
				
				if best_j.nil?
					$stderr.write " -> NO CORR. \n" if DESCRIBE
				else	
					$stderr.write " -> #{best_j} (#{i-best_j})\n" if DESCRIBE
				end
			end
			
#			we_start_at = start_cell+last_diff;
			we_start_at = last_best.nil? ? start_cell : last_best
			
			we_start_at = [from, we_start_at].max
			we_start_at = [to, we_start_at].min
			
			up =  we_start_at+1; up_stopped = false;
			down = we_start_at; down_stopped = false;
			last_dist_up = -1; # first is up
			last_dist_down = 0;	
			best = nil
			best_dist = 1000
			$stderr.write "[from #{from} d #{down} s #{start_cell} u #{up} to #{to}]" if DESCRIBE

		#	now_up = false
 			while (not up_stopped) or (not down_stopped)
				dbg_num_a1 += 1
				
				now_up =   up_stopped ? false : 
				         down_stopped ? true  : last_dist_up < last_dist_down
#						now_up = now_up ? false : true
				
				$stderr.write " |" if DESCRIBE
				
				if now_up 
					$stderr.write "up=#{up} " if DESCRIBE
					if up > to then up_stopped = true; next end
					if @p_j[up].nil? then next end
					last_dist_up = (p_i_w - @p_j[up]).nrm2
					if last_dist_up < best_dist
						$stderr.write "*" if DESCRIBE
						best = up; best_dist = last_dist_up;
					end
					delta_theta = ([up-start_cell,0].max).abs * (PI/laser_ref.nrays)
					min_dist_up = sin(delta_theta) * p_i_w_nrm2
					if min_dist_up > best_dist then 
						$stderr.write "stop" if DESCRIBE
						up_stopped = true
						next 
					end
					if JUMP_TRICK && (up>start_cell)
						if @p_j[up].nrm2 < p_i_w_nrm2 && 
							(not @up_bigger[up].nil?) then
							$stderr.write " J+(#{@up_bigger[up]})" if DESCRIBE
							up += @up_bigger[up]
							next
						end
				
						if @p_j[up].nrm2 > p_i_w_nrm2 && 
							(not @up_smaller[up].nil?) then
							$stderr.write " J-(#{@up_smaller[up]})" if DESCRIBE
							up += @up_smaller[up]
							next
						end
					end
					up+= 1
				else
					$stderr.write "dn=#{down} " if DESCRIBE
					if down < from then down_stopped = true; next end
					if @p_j[down].nil? then next end
					last_dist_down = (p_i_w - @p_j[down]).nrm2
					if last_dist_down < best_dist
						$stderr.write "*" if DESCRIBE
						best = down; best_dist = last_dist_down;
					end
					delta_theta = ([start_cell-down,0].max).abs*(PI/laser_ref.nrays)
					min_dist_down = sin(delta_theta) * p_i_w_nrm2
					if min_dist_down > best_dist then 
						$stderr.write "stop" if DESCRIBE
						down_stopped = true
						next 
					end
					if JUMP_TRICK && (down<start_cell)
						if @p_j[down].nrm2+JUMP_TRICK_DELTA < p_i_w_nrm2 &&
							(not @down_bigger[down].nil?) then
							$stderr.write " J+(#{@down_bigger[down]})" if DESCRIBE
							down += @down_bigger[down]
							next
						end
				
						if @p_j[down].nrm2 > JUMP_TRICK_DELTA+p_i_w_nrm2 &&
							(not @down_smaller[down].nil?) then
							$stderr.write " J-(#{@down_smaller[down]})" if DESCRIBE
							down += @down_smaller[down]
							next
						end
					end
					down-= 1
				end
			end
			
			last_diff = best - start_cell
			last_best = best;
			
			puts "\n#{i} --> #{best} " if DESCRIBE
			
			if (not best_j.nil?) and (best!=best_j)
				puts "\n*******\n\n#{i} --> bf #{best_j} (dist #{best_j_dist}) != #{best} (dist #{best_dist})" 
			
				for j in ([best_j-15,0].max)..([best_j+15,laser_ref.nrays].min)
					puts "ray #{j} reading #{@p_j[j].nil? ? 'nil' : @p_j[j].nrm2}"+
					" upBig #{@up_bigger[j]} upS #{@up_smaller[j]} "+
					" dnBig #{@down_bigger[j]} dnS #{@down_smaller[j]} "
				end
				puts "\n\n"
				exit
			end
			best_j = best
			
			## If no point is close enough, or closest point is one 
			## of the extrema, then discard
			if [nil, 0, laser_ref.nrays-1].include? best_j
				@correspondences[i] = nil
				next
			end

			## Find other point to interpolate. See if we are closed
			## to prev or next
			
			# FIXME: here we assume that all points are valid
			p_prev = laser_ref.points[best_j-1].cartesian;
			p_next = laser_ref.points[best_j+1].cartesian;
			dist_prev = (p_prev-p_i_w).nrm2;
			dist_next = (p_next-p_i_w).nrm2;
			other_j =
				if dist_prev < dist_next then best_j-1 else best_j+1 end
			
			c = Correspondence.new
			c.i = i;
			c.j1 = best_j;
			c.j2 = other_j;
			@correspondences[i] = c

		#	puts "i = #{i}, j1 = #{best_j}, j2 = #{other_j}"
			
		end # i in first scan 
	#	puts "j: " + @correspondences.map { |c| c.nil? ? nil: c.j1 }.join(",")

		puts "Total number: #{dbg_num_a1}"

	end
	
	require 'point2line'

	def compute_next_estimate(x_old)
		
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		## current distances
		dists = Array.new
		@total_error = 0
		corrs = Array.new
		for c in @correspondences; next if c.nil?
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
