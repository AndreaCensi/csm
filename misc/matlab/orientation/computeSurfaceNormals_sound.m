% params.sigma
% params.curv 
% params.max_points
% params.threshold
% params.min_dist

function ld = computeSurfaceNormals_sound(ld)

	params.max_points=10; 
	params.threshold=2;
	params.min_dist = 0.24;
	params.curv = 0;
	params.sigma = 0.01;
	
curv = params.curv;
var = params.sigma ^ 2;
MAX = params.max_points;
MIN_DIST = params.min_dist;
THRESHOLD = params.threshold;

n = size(ld.points,2);

for i=1:n


	if i==1 
		min_dist = norm(ld.points(:,i)-ld.points(:,i+1));
	else
		if i==n
			min_dist = norm(ld.points(:,i)-ld.points(:,i-1));
		else
			min_dist = min( norm(ld.points(:,i)-ld.points(:,i-1)), ...
								 norm(ld.points(:,i)-ld.points(:,i+1)));
		end
	end
	
	min_dist = max(min_dist,MIN_DIST);
	
	imax=i;
	% vediamo quanto allungare
	for j=min(i+1,n):min(i+MAX,n)
		d = norm( ld.points(:,j)- ld.points(:,j-1) );
		if d > THRESHOLD * min_dist
			break;
		end
		imax = j;
	end
	
	imin=i;
	for j=max(1,i-1):-1:max(1,i-MAX)
		d = norm( ld.points(:,j)- ld.points(:,j+1) );
		if d > THRESHOLD * min_dist
			break;
		end
		imin=j;
	end
	
%	imin = max(1, i-3);
%	imax = min(n, i+3);
	
	ni=imax-imin+1;

	if ni>2 
		indexes=[imin:i-1 i+1:imax];
		
		t = ld.theta(indexes);
		f = ld.readings(indexes);
		fvar = ones(1,ni) * var;
		fd_guess = zeros(1,ni);
		
		t0 = ld.theta(i);
		f0 = ld.readings(i);
		fvar0 = var;
		fd_guess0 = 0;
		
		curvf = 0;
		
		[fd0,fd0_var] = estimate_derivative_single( ...
			t,f,fvar,fd_guess, ...
			t0,f0,fvar0,fd_guess0,curvf);

		alpha = t0 - atan(fd0/f0);
		curvf = curv *(abs(f0 * tan(t0-alpha))^2);
				
		[fd0,fd0_var] = estimate_derivative_single( ...
			t,f,fvar,fd_guess, ...
			t0,f0,fvar0,fd_guess0,curvf);

		
		theta = t0;
		
		%q = fd0 / f0;
		%alpha = atan(  (cos(theta)+q*sin(theta)) / (-sin(theta)+q*cos(theta)) );
		
		alpha = theta - atan(fd0/f0);
		alpha_var =  fd0_var * (( f0 / (f0.^2 + fd0.^2) ).^2) ...
		            + var *  (( fd0 / (f0.^2 + fd0.^2) ).^2) ;
		
		ld.alpha(i) =  alpha-pi; % rivolta verso dentro
		ld.alpha_valid(i) = 1;
		ld.alpha_error(i) = alpha_var;
		
		
		indexes=[imin:imax];
		[ld.ar_alpha(i), ld.ar_alpha_error(i)] = ...
			arras_fit( ld.theta(indexes), ld.readings(indexes), var);
	else
		ld.alpha(i) = nan;
		ld.alpha_valid(i) = 0;
		ld.alpha_error(i) = nan;
	end
	
end

