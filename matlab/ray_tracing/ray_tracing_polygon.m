% polylist = { [p0;p1], [p1;p2] }
function ld = ray_tracing_polygon(seglist, pose, nrays, fov, max_reading)

	t = pose(1:2);
	ld.nrays = nrays;
	for i=1:nrays
		theta = -fov/2 + fov * (i-1)/(nrays-1);
		ld.theta(i) = theta;

		[valid, reading, alpha] = try_segment_list(seglist, pose(1:2), pose(3)+theta, max_reading);
		
		if valid
			ld.readings(i) = reading;
			ld.readings_valid(i) = true;
			ld.true_alpha(i) = alpha-pose(3);
			ld.true_alpha_abs(i) = alpha;
		else
			ld.readings(i) = nan;
			ld.readings_valid(i) = false;
			ld.true_alpha(i) = nan;
			ld.true_alpha_abs(i) = nan;
		end

		ld.true_alpha(i) = alpha-pose(3); % local coordinates
		ld.true_alpha_abs(i) = alpha; % local coordinates
		
		if rand>0.95
			pause(0.02)
		end
	end
	
	
	ld.odometry = pose;
	ld.estimate = pose;
	ld.timestamp1 = '0';
	ld.timestamp2 = '0';
	ld.hostname = 'matlab';
	ld.points = [ cos(ld.theta) .* ld.readings; sin(ld.theta).* ld.readings];
	
function [valid, reading, alpha] = try_segment_list(seglist, position, direction, max_reading)
	valid = false;
	reading = nan;
	alpha = nan;
	for i=1:size(seglist,2)
		AB = seglist{i};
		[valid_i, reading_i, alpha_i] = try_segment(AB(:,1),AB(:,2),position, direction, max_reading);
		better = valid_i & ( (~valid) | (reading_i < reading));
		if better
			valid = true;
			reading = reading_i;
			alpha = alpha_i;
		end
	end

function [valid, reading, alpha] = try_segment(A, B, position, direction, max_reading)
	valid = false;
	reading = nan;
	alpha = nan;

	% Direction of view
	N = vers(direction);
	
	% normal to segment
	S = rot(pi/2) * (A-B);
	
	epsilon = 0.0000001;
	if abs(S'*N) < epsilon
		return;
	end
	
	dist = (S'*(A-position)) / (S'*N);
	if dist < 0
		return
	end

	% Now we check whether the crossing point
	% with the line lies within the segment
	crossingPoint = position + N * dist;
	% distance from segment center to crossing point
	rad = norm(crossingPoint - ( 0.5*A + 0.5*B));
	if rad > norm(A-B)/2  
		return;
	end		
	
	if(max_reading == 0) | (dist < max_reading)
		reading = dist;
		valid = true;
		alpha = atan2(S(2),S(1));
		if vers(alpha)' * N > 0
			alpha = alpha + pi;
		end
		if alpha > 2*pi
			alpha = alpha - 2*pi;	
		end
	end
	
