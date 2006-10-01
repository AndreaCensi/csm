
class ICP
	DESCRIBE = false
	VERIFY_EXACT = false
	JUMP_TRICK = true
	JUMP_TRICK_DELTA = 0

	# log search into journal
	JOURNAL_SEARCH = false
	
	def find_correspondences_tricks(x_old)
		# Some parameters
		maxAngularCorrectionDeg = params[:maxAngularCorrectionDeg]
		maxLinearCorrection=params[:maxLinearCorrection]
		maxDist = params[:maxCorrespondenceDist]	
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		# If true, we compare the result with the slow algorithm, for correctness
		if VERIFY_EXACT
			exact = find_correspondences(x_old)
		end

		p_j = Array.new
		for j in 0..params[:laser_ref].nrays-1
			if params[:laser_ref].points[j].valid? 
				p_j[j] = params[:laser_ref].points[j].cartesian
			end
		end

		correspondences = Array.new
	
		up_bigger, up_smaller, down_bigger, down_smaller = 
			create_jump_tables(laser_ref)
			
		dbg_num_a1 = 0
		last_best = nil
		for i in 0..laser_sens.nrays-1
			if not laser_sens.points[i].valid?
				correspondences[i] = nil
				next
			end
		
			p_i = laser_sens.points[i].cartesian
			p_i_w = transform(p_i, x_old);
			p_i_w_nrm2 = p_i_w.nrm2

			from, to, start_cell = 
				possible_interval(p_i_w, laser_sens,
					maxAngularCorrectionDeg, maxLinearCorrection)
	
			we_start_at = last_best.nil? ? start_cell : last_best
		
			we_start_at = [from, we_start_at].max
			we_start_at = [to,   we_start_at].min
		
			up =  we_start_at+1; up_stopped = false;
			down = we_start_at; down_stopped = false;
			last_dist_up = -1; # first is up
			last_dist_down = 0;	
			best = nil
			best_dist = 1000
			
			@js_string = ""
			def js(s)
				@js_string << s
			#	$stderr.puts '>>>>>>>>>' if @js_string.size == 0
			#	$stderr.write s
			end
			
			js  "[from #{from} down #{down} mid #{start_cell} up #{up} to #{to}]"
			
			while (not up_stopped) or (not down_stopped)
			
				now_up =   up_stopped ? false : 
				         down_stopped ? true  : last_dist_up < last_dist_down
			
				js(" |")
			
				if now_up 
					js "up=#{up} " 
					if up > to then
						js "down:stop" 
						up_stopped = true; next 
					end
					if p_j[up].nil? then 
						js "invalid" 
						up+=1
						next 
					end
			
					dbg_num_a1 += 1
					last_dist_up = (p_i_w - p_j[up]).nrm2
					
					if (last_dist_up<maxDist) && (last_dist_up < best_dist)
						js  "*" 
						best = up; best_dist = last_dist_up;
					end
					
					if JUMP_TRICK && (up>start_cell)
						if p_j[up].nrm2 < p_i_w_nrm2 && 
							(not up_bigger[up].nil?) then
							js " J+(#{up_bigger[up]})" 
							up += up_bigger[up]
							next
						else
						if p_j[up].nrm2 > p_i_w_nrm2 && 
							(not up_smaller[up].nil?) then
							js  " J-(#{up_smaller[up]})"
							up += up_smaller[up]
							next
						else
							up+=1
						end
						end
					else
						up+= 1
					end
					
					delta_theta = ([up-start_cell,0].max).abs * (PI/laser_ref.nrays)
					min_dist_up = sin(delta_theta) * p_i_w_nrm2
					if min_dist_up > best_dist then 
						js  "up:early_stop"
						up_stopped = true
						next 
					end
					
				else
					js  "dn=#{down} " 
					if down < from then
						js "down:stop" 
						down_stopped = true; 
						next 
					end
					if p_j[down].nil? then 
						js "invalid" 
						down-=1; 
						next 
					end

					dbg_num_a1 += 1
					last_dist_down = (p_i_w - p_j[down]).nrm2
					if (last_dist_down<maxDist) && (last_dist_down < best_dist)
						js "*"
						best = down; 
						best_dist = last_dist_down;
					end

					delta_theta = ([start_cell-down,0].max).abs*(PI/laser_ref.nrays)
					min_dist_down = sin(delta_theta) * p_i_w_nrm2
					if min_dist_down > best_dist then 
						js  "down:early_stop"
						down_stopped = true
						next 
					end
					
					if JUMP_TRICK && (down<start_cell)
						if p_j[down].nrm2+JUMP_TRICK_DELTA < p_i_w_nrm2 &&
							(not down_bigger[down].nil?) then
							js " Jump+(#{down_bigger[down]})"
							down += down_bigger[down]
						else
							if p_j[down].nrm2 > JUMP_TRICK_DELTA+p_i_w_nrm2 &&
								(not down_smaller[down].nil?) then
								js " Jump-(#{down_smaller[down]})"
								down += down_smaller[down]
								next
							else 
								down-= 1
							end
						end
					else
						js "$" 
						down-= 1
					end
					
				end
			end
		
			last_best = best;
		
			js " FINAL #{i} --> #{best} " 
		
			if JOURNAL_SEARCH
				journal "tricks #{i}   #{@js_string}"
			end
			
			#
			## If no point is close enough, or closest point is one 
			## of the extrema, then discard
			if [nil, 0, laser_ref.nrays-1].include? best
				correspondences[i] = nil
				next
			else
				## Find other point to interpolate. See if we are closed
				## to prev or next
		
				# FIXME: here we assume that all points are valid
				p_prev = laser_ref.points[best-1].cartesian;
				p_next = laser_ref.points[best+1].cartesian;
				dist_prev = (p_prev-p_i_w).nrm2;
				dist_next = (p_next-p_i_w).nrm2;
				other_j =
					if dist_prev < dist_next then best-1 else best+1 end
		
				c = Correspondence.new
				c.i = i;
				c.j1 = best;
				c.j2 = other_j;
				correspondences[i] = c
			end

			# This checks whether the solution is the same without tricks
			if VERIFY_EXACT
				error = false
				if exact[i] == correspondences[i]
					next
				end
				
				if exact[i].nil? and (not correspondences[i].nil?)
					$stderr.puts "He did not find a correspondence!"
					puts "exact = " + (exact.map{|c| c.nil? ? 'NIL': c.j1 }.join(','))
					
					puts "i = #{i}, best = #{correspondences[i].j1}, dist = #{best_dist}"
					puts "#{js}"
					exit
				end
				
				if (not exact[i].nil?) and correspondences[i].nil?
					$stderr.puts "I did not find a correspondence!"
					exit
				end
				
				if exact[i].j1 != correspondences[i].j1
					
					 my_corr = laser_ref.points[correspondences[i].j1].cartesian
					 my_dist =  (my_corr - p_i_w).nrm2
					his_corr = laser_ref.points[exact[i].j1].cartesian
					his_dist = (his_corr - p_i_w).nrm2
					
					if his_dist < my_dist
						$stderr.puts "Optimal is #{exact[i].j1} (dist= #{his_dist}), " +
							", while I found #{correspondences[i].j1} (dist = #{my_dist})"
						$stderr.puts "Search was: #{@js_string}"
					end

#					for j in ([best_j-15,0].max)..([best_j+15,laser_ref.nrays].min)
#						puts "ray #{j} reading #{@p_j[j].nil? ? 'nil' : @p_j[j].nrm2}"+
#						" upBig #{@up_bigger[j]} upS #{@up_smaller[j]} "+
#						" dnBig #{@down_bigger[j]} dnS #{@down_smaller[j]} "
#					end

				end
			end

		end # i in first scan 

		puts "Total number: #{dbg_num_a1}"
		correspondences
	end
	
	
	def create_jump_tables(laser_ref)
		up_bigger = Array.new
		up_smaller = Array.new
		down_bigger = Array.new
		down_smaller = Array.new
		readings = (0..laser_ref.nrays-1).map{ |j| laser_ref.points[j].reading}
		delta = 0;
		readings.each_index{ |i|
			j = i + 1;
			while (not readings[j].nil?) and 
				readings[j]<=readings[i]+JUMP_TRICK_DELTA
				j += 1;
			end
			up_bigger[i] = j-i;

			j = i + 1;
			while (not readings[j].nil?) and 
				readings[j]+JUMP_TRICK_DELTA>=readings[i]
				j += 1;
			end
			up_smaller[i] = j-i;

			j = i - 1;
			while (not readings[j].nil?) and 
				readings[j]+JUMP_TRICK_DELTA>=readings[i]
				j -= 1;
			end
			down_smaller[i] = j-i;
	
			j = i - 1;
			while (not readings[j].nil?) and 
				readings[j]<=readings[i]+JUMP_TRICK_DELTA
				j -= 1;
			end
			down_bigger[i] = j-i;			
		}
		
		
		if JOURNAL_SEARCH
			journal "  down bigger:  #{down_bigger. join(',')}"
			journal " down smaller: #{down_smaller.join(',')}"
			journal "    up bigger:  #{  up_bigger .join(',')}"
			journal "   up smaller: #{  up_smaller.join(',')}"
		end
	
		return up_bigger, up_smaller, down_bigger, down_smaller
	end
	
end