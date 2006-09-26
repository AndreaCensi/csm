require 'gsl'
require 'structures'

module GSL
	def deg2rad(d); d * Math::PI / 180; end
	def rad2deg(d); d / Math::PI * 180; end
end

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
		param :epsilon_theta,  deg2rad(0.01)
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
		for iteration in 1..params[:maxIterations]
			puts "Iteration #{iteration}"
		
			find_correspondences(x_old)
		
			x_new = compute_next_estimate

#			delta = Pose.minus(x_new, x_old);
			delta = x_new - x_old
			
			puts "x_new = #{x_new}"
			puts "delta = #{delta} eps = #{ params[:epsilon_xy] }"
		
			if delta[0,1].nrm2 < params[:epsilon_xy] &&
			   delta[2].abs < params[:epsilon_theta]
				
				break
			end
			
			x_old = x_new;
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
		if false
		diffs = (1..laser_ref.nrays-1).map{ |j| (laser_ref.points[j].reading- 
			laser_ref.points[j-1].reading).abs}
		
		diffs_big = diffs.map{|x| x>1 ? x : nil}
		diffs_small = diffs.map{|x| x<1 ? x : nil}
		
		diffs_big_up = (0..laser_ref.nrays-2).map{ |j| 
			diffs_big[j,j+30].compact.min
		}
		diffs_small_up = (0..laser_ref.nrays-2).map{ |j| 
			diffs_small[j,j+30].compact.max
		}
		puts "diff_bi = #{diffs_big.join(',')}"
		puts "diff_sm = #{diffs_small.join(',')}"
		puts "diff_bi_up = #{diffs_big_up.join(',')}"
		puts "diff_sm_up = #{diffs_small_up.join(',')}"
		end

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
		
			## Find best correspondence
			best_j = nil;
			if false
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
				$stderr.write " -> #{best_j} (#{i-best_j})\n" if DESCRIBE
			end
			
#			we_start_at = start_cell+last_diff;
			we_start_at = last_best.nil? ? start_cell : last_best
			up = we_start_at-1; up_stopped = false;
			down = we_start_at; down_stopped = false;
			last_dist_up = -1; # first is up
			last_dist_down = 0;	
			best = nil
			best_dist = 1000
			$stderr.write "[#{start_cell}]" if DESCRIBE

		#	now_up = false
 			while (not up_stopped) or (not down_stopped)
				dbg_num_a1 += 1
				
				if up_stopped then now_up = false else
					if down_stopped then now_up = true else
						
						now_up = last_dist_up<last_dist_down
#						now_up = now_up ? false : true
					end
				end
					
				if now_up 
					next if up_stopped
					up += 1
					$stderr.write " + #{up} " if DESCRIBE
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
						$stderr.write "stop up " if DESCRIBE
						up_stopped = true
						next 
					end
				else
					next if down_stopped
					down -= 1
					$stderr.write " - #{down} " if DESCRIBE
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
						$stderr.write "stop down" if DESCRIBE
						down_stopped = true
						next 
					end
				end
			end
			
			last_diff = best - start_cell
			last_best = best;
			
			puts "\n#{i} --> #{best} " if DESCRIBE
			
			if (not best_j.nil?) and (best!=best_j)
				puts "\n#{i} --> #{best_j} != #{best} (#{best_j_dist}, #{best_dist})" 
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
		puts "j: " + @correspondences.map { |c| c.nil? ? nil: c.j1 }.join(",")

		puts "#{dbg_num_a1}"
		
	end
	
	def compute_next_estimate
		Vector.alloc(0,0,0)
	end
	
	
end
