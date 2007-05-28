
class ICP
	
	def find_correspondences(x_old)
		correspondences = Array.new
		max_angular_correction_deg = params[:max_angular_correction_deg]
		max_linear_correction=params[:max_linear_correction]
		maxDist = params[:max_correspondence_dist]
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		for i in 0..laser_sens.nrays-1
			if not laser_sens.valid? i 
				laser_sens.set_null_corr(i)
				next
			end

			p_i = laser_sens.p[i]
			p_i_w = transform(p_i, x_old);
			p_i_w_nrm2 = p_i_w.nrm2

		#	puts "p_i = #{p_i.trans}"
			from, to = 
				possible_interval(p_i_w, laser_sens, 
					max_angular_correction_deg, max_linear_correction)

			## Find best correspondence by considering all points in (from, to)
			best_j = nil;
			best_j_dist = 0;
			$stderr.write "#{i}: " if DESCRIBE
			for j in from..to
				$stderr.write "#{j}" if DESCRIBE
				if not laser_ref.valid? j
					$stderr.write "N" if DESCRIBE
					next
				end

				## Find compatible interval in the other scan. 
				dist = (p_i_w - laser_ref.p[j]).nrm2
		#		puts "j = #{j} p_j = #{p_j[j].trans}  dist = #{dist}"
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
#				puts " #{i} -> #{best_j} (#{best_j_dist}) p_j = #{p_j[best_j].trans} p_i_w = #{p_i_w.trans}" 
			end

			#
			## If no point is close enough, or closest point is one 
			## of the extrema, then discard
			if [nil, 0, laser_ref.nrays-1].include? best_j
				laser_sens.set_null_corr(i)
				next
			else
				## Find other point to interpolate. See if we are closed
				## to prev or next
		
				# FIXME: here we assume that all points are valid
				j_up = laser_ref.next_valid_up(best_j)
				j_down = laser_ref.next_valid_down(best_j)
				if j_up.nil? and j_down.nil?
					laser_sens.set_null_corr(i)
				else
					other_j =
					if j_up.nil? or j_down.nil?
						[j_up,j_down].compact[0]
					else	
						p_prev = laser_ref.p[j_up]
						p_next = laser_ref.p[j_down]
						dist_prev = (p_prev-p_i_w).nrm2;
						dist_next = (p_next-p_i_w).nrm2;
						if dist_prev < dist_next then j_down else j_up end
					end
					laser_sens.corr[i].valid = true
					laser_sens.corr[i].j1 = best_j
					laser_sens.corr[i].j2 = other_j
				end
			end
		end # i in first scan 
	end

	
	
	
	
	
	
end