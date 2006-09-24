
function [P,valid,jindexes] = icp_get_correspondences(params,current_estimate)
	debug = 0;
	for i=1:params.laser_sens.nrays
		p_i = params.laser_sens.points(:,i);
		p_i_w = transform(p_i, current_estimate);

		[from, to] = icp_possible_interval(p_i_w, params.laser_ref,...
			params.maxAngularCorrectionDeg, params.maxLinearCorrection);
		
		%% Find best correspondence
		best_j = 0; best_dist = 0;
		for j=from:to
			% Find compatible interval in the other scan. 
			p_j = params.laser_ref.points(:,j);
			
			dist = norm( p_i_w - p_j);
			if dist < params.maxCorrespondenceDist
				if (best_j==0) || (dist < best_dist)
					best_j = j; best_dist = dist;
				end
			end
		end

		if (best_j == 0) || ...
			(params.dont_consider_extrema && ...
				((best_j==1) || (best_j==params.laser_ref.nrays)))
			P(:,i) = [nan;nan];
			valid(i) = 0;
			jindexes(i,:)=[nan;nan];
		else
			%% Find other point to interpolate
			if best_j==1
				other_j = best_j + 1;
				other = params.laser_ref.points(:,best_j+1);
			elseif best_j == params.laser_ref.nrays
				other_j = best_j - 1;
				other = params.laser_ref.points(:,best_j-1);
			else
				p_prev = params.laser_ref.points(:,best_j-1);
				p_next = params.laser_ref.points(:,best_j+1);
				dist_prev = norm( p_prev-p_i_w);
				dist_next = norm( p_next-p_i_w);
				if dist_prev < dist_next
					other_j = best_j -1;
					other = p_prev;
				else
					other_j = best_j +1;
					other = p_next;
				end
			end
			
			%% Find the point which is closest to segment.
			interpolate = closest_point_on_segment(...
				params.laser_ref.points(:,best_j),other, p_i_w);
	
			dist = norm(interpolate-p_i_w);
			if dist < params.maxCorrespondenceDist
				P(:,i) = interpolate;
				valid(i) = 1;
				jindexes(i,:) = [best_j;other_j];
			else
				P(:,i) = [nan;nan];
				valid(i) = 0;
				jindexes(i,:) = [nan;nan];
			end
		end
			
	end % i in first scan 
	
