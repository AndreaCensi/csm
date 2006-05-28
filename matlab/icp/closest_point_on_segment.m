
function res = closest_point_on_segment(A,B,p)
% closest_point_on_segment(A,B,p)
%  find closest point to p on segment A-B
	projection = projection_on_line_seg(A,B,p);
	
%	fprintf('Closest(%s,%s;%s)\n', pv(A), pv(B), pv(p));
%	fprintf('  projection: \n', pv(projection));
	
%	fprintf('A: %s B: %s p: %s proj: %s\n',pv(A),pv(B),pv(p),pv(projection));

	%res = projection;
	%return;
		
	% check whether projection is inside the segment
	if (projection-A)'*(projection-B)<0
		res = projection;
	else
		if norm(p-A) < norm(p-B)
			res = A;
		else
			res = B;
		end
	end
		
function res = projection_on_line_seg(A,B,p)
% projection_on_line_seg(A,B,p)
%  finds projection of p on line through A,B

	% find polar representation
	v_alpha = rot(pi/2) * (A-B) / norm(A-B);
	alpha = atan2(v_alpha(2),v_alpha(1));
	rho = v_alpha' * A; 
	res = projection_on_line(alpha, rho, p);
%	fprintf('alpha = %f  v_alpha = %s  rho = %f\n', alpha, pv(v_alpha), rho);

function res = projection_on_line(alpha, rho, p)
% projection_on_line(alpha, rho, p)
%  finds projection of p on line whose polar representation is (alpha,rho)

	res = vers(alpha) * rho + (p-(vers(alpha)'*p)*vers(alpha));
	
	% 0 == vers(alpha)' * res - rho


