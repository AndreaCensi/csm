
class ICP
	DESCRIBE = false
	VERIFY_EXACT = true
	JUMP_TRICK = true
	
	# log search into journal
	JOURNAL_SEARCH = false
	
	def find_correspondences_tricks(x_old)
		# Some parameters
		max_angular_correction_deg = params[:max_angular_correction_deg]
		max_linear_correction=params[:max_linear_correction]
		maxDist = params[:max_correspondence_dist]	

		journal "param max_correspondence_dist #{maxDist}"
		journal "param max_angular_correction_deg #{max_angular_correction_deg}"
		journal "param max_linear_correction #{max_linear_correction}"

		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		down_bigger  = laser_sens.down_bigger
		up_bigger    = laser_sens.up_bigger
		down_smaller = laser_sens.down_smaller
		up_smaller   = laser_sens.up_smaller
		
		
		# If true, we compare the result with the slow algorithm, for correctness
		if VERIFY_EXACT
			find_correspondences(x_old)
			exact = laser_sens.corr.clone
		end
		
		dbg_num_a1 = 0
		last_best = nil
		for i in 0..laser_sens.nrays-1
			if not laser_sens.valid? i 
				laser_sens.set_null_corr(i)
				next
			end
		
			p_i = laser_sens.p[i]
			p_i_w = transform(p_i, x_old);
			p_i_w_nrm2 = p_i_w.nrm2

			from, to, start_cell = 
				possible_interval(p_i_w, laser_sens,
					max_angular_correction_deg, max_linear_correction)
	
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
			
				if now_up then                                            js "up=#{up} " 
					if up > to then                                       js "down:stop" 
						up_stopped = true; next 
					end
					if laser_ref.p[up].nil? then                                  js "invalid" 
						up+=1; next 
					end
			
					dbg_num_a1 += 1
					last_dist_up = (p_i_w - laser_ref.p[up]).nrm2
					
					if (last_dist_up<maxDist) && (last_dist_up < best_dist) then     js  "*" 
						best = up; best_dist = last_dist_up;
					end
					
					if JUMP_TRICK && (up>start_cell)
						if laser_ref.readings[up] < p_i_w_nrm2 && 
							(not up_bigger[up].nil?) then                  js " J+(#{up_bigger[up]})" 
							up += up_bigger[up]
							next
						else
						if laser_ref.readings[up] > p_i_w_nrm2 && 
							(not up_smaller[up].nil?) then                 js  " J-(#{up_smaller[up]})"
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
					if min_dist_up > best_dist then                       js  "up:early_stop"
						up_stopped = true
						next 
					end
					
				else                                                     js  "dn=#{down} " 
					if down < from then
						js "down:stop" 
						down_stopped = true; 
						next 
					end
					if not laser_ref.valid? down then 
						js "invalid" 
						down-=1; 
						next 
					end

					dbg_num_a1 += 1
					last_dist_down = (p_i_w - laser_ref.p[down]).nrm2
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
						if laser_ref.readings[down] < p_i_w_nrm2 &&
							(not down_bigger[down].nil?) then
							js " Jump+(#{down_bigger[down]})"
							down += down_bigger[down]
						else
							if laser_ref.readings[down] >  p_i_w_nrm2 &&
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
				laser_sens.set_null_corr(i)
				next
			else
				## Find other point to interpolate. See if we are closed
				## to prev or next
		
				# FIXME: here we assume that all points are valid
				j_up = laser_ref.next_valid_up(best)
				j_down = laser_ref.next_valid_down(best)
				if j_up.nil? and j_down.nil?
					laser_sens.set_null_corr(i)
				else
					other_j =
					if j_up.nil? or j_down.nil?
						[j_up,j_down].compact[0]
					else	
						p_prev = laser_ref.p[j_down]
						p_next = laser_ref.p[j_up]
						dist_prev = (p_prev-p_i_w).nrm2;
						dist_next = (p_next-p_i_w).nrm2;
						if dist_prev < dist_next then j_down else j_up end
					end
					laser_sens.corr[i].valid = true
					laser_sens.corr[i].j1 = best
					laser_sens.corr[i].j2 = other_j
				end
			end

			# This checks whether the solution is the same without tricks
			if VERIFY_EXACT
				
				if (not exact[i].valid) and (laser_sens.corr[i].valid)
					$stderr.puts "He did not find a correspondence!"
					puts "exact = " + (exact.map{|c| c.nil? ? 'NIL': c.j1 }.join(','))
					
					puts "i = #{i}, best = #{correspondences[i].j1}, dist = #{best_dist}"
					puts "#{js}"
					exit
				end
				
				if (exact[i].valid) and (not laser_sens.corr[i].valid)
					$stderr.puts "I did not find a correspondence!"
					exit
				end
				
				if exact[i].j1 != laser_sens.corr[i].j1
					
					 my_corr = laser_ref.p[j1]
					 my_dist =  (my_corr - p_i_w).nrm2
					his_corr = laser_ref.p[exact[i].j1]
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

	#	puts "Total number: #{dbg_num_a1}"
	end
end
	
class LaserData	
	def create_jump_tables
		
		for i in 0..nrays-1
			j = i + 1;
			while (valid? j) and readings[j]<=readings[i]
				j += 1;
			end
			@up_bigger[i] = j-i;

			j = i + 1;
			while (valid? j) and readings[j]>=readings[i]
				j += 1;
			end
			@up_smaller[i] = j-i;

			j = i - 1;
			while (valid? j) and readings[j]>=readings[i]
				j -= 1;
			end
			@down_smaller[i] = j-i;
	
			j = i - 1;
			while (valid? j) and readings[j]<=readings[i]
				j -= 1;
			end
			@down_bigger[i] = j-i;
		end
	end	
=begin
		if JOURNAL_SEARCH
			journal "down_bigger #{@down_bigger. join(' ')}"
			journal "down_smaller #{@down_smaller.join(' ')}"
			journal "up_bigger #{  @up_bigger .join(' ')}"
			journal "up_smaller #{  @up_smaller.join(' ')}"
		end
=end
end