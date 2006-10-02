
class ICP
	
	# from, to = 
	def possible_interval(point, ld, maxAngularCorrectionDeg, 
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
	
	
	
	def find_correspondences(x_old)
		correspondences = Array.new
		maxAngularCorrectionDeg = params[:maxAngularCorrectionDeg]
		maxLinearCorrection=params[:maxLinearCorrection]
		maxDist = params[:maxCorrespondenceDist]
		laser_ref = params[:laser_ref];
		laser_sens = params[:laser_sens];

		p_j = Array.new
		for j in 0..params[:laser_ref].nrays-1
			if params[:laser_ref].points[j].valid? 
				p_j[j] = params[:laser_ref].points[j].cartesian
			end
		end
		
		for i in 0..laser_sens.nrays-1
			if not laser_sens.points[i].valid?
				correspondences[i] = nil
				next
			end

			p_i = laser_sens.points[i].cartesian
			p_i_w = transform(p_i, x_old);
			p_i_w_nrm2 = p_i_w.nrm2

		#	puts "p_i = #{p_i.trans}"
			
			from, to = 
				possible_interval(p_i_w, laser_sens, 
					maxAngularCorrectionDeg, maxLinearCorrection)

			from = 0; to = laser_ref.nrays
			## Find best correspondence by considering all points in (from, to)
			best_j = nil;
			best_j_dist = 0;
			$stderr.write "#{i}: " if DESCRIBE
			for j in from..to
				$stderr.write "#{j}" if DESCRIBE
				if p_j[j].nil?
					$stderr.write "N" if DESCRIBE
					next
				end

				## Find compatible interval in the other scan. 
				dist = (p_i_w - p_j[j]).nrm2
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

			## If no point is close enough, or closest point is one 
			## of the extrema, then discard
			if [nil, 0, laser_ref.nrays-1].include? best_j
				correspondences[i] = nil
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
			correspondences[i] = c
		end # i in first scan 

		correspondences
	end

	
	
	
	
	
	
end