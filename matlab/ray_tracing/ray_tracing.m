function ld = ray_tracing(pose, fov, nrays, countour, countour_params, precision)

	t = pose(1:2);
	ld.nrays = nrays;
	for i=1:nrays
		ld.theta(i) = -fov/2 + fov * (i-1)/(nrays-1);
		params = {t, pose(3)+ld.theta(i), countour, countour_params};
		tau = rfBisection( 'eval_delta', params, 0,0.99,precision);
		[point, alpha] = feval(countour, countour_params, tau);
		ld.readings(i) = norm(point-t);
		ld.true_alpha(i) = alpha-pose(3); % local coordinates
	end
	
	ld.odometry = pose;
	ld.estimate = pose;
	ld.timestamp1 = '0';
	ld.timestamp2 = '0';
	ld.hostname = 'matlab';
	ld.points = [ cos(ld.theta) .* ld.readings; sin(ld.theta).* ld.readings]';
	
function delta = eval_delta(params, tau)
	
	% secondo parametro: t
	t = params{1};
	theta = params{2};
	curve = params{3};
	curve_params = params{4};
	point = feval(curve, curve_params, tau);
	phi = angle(point-t);
	delta = normAngle(phi-theta);

	fprintf('tau = %f  point = %f %f theta=%f phi=%d delta=%f\n', ...
		tau, point(1), point(2), theta, phi, delta);
	
function a = angle(v)
	a = atan2(v(2),v(1));

function root = rfBisection( func, func_params, a, b, epsilon)

    f_a = feval(func, func_params, a);
    f_b = feval(func, func_params, b);
    
    s = sign(f_a) * sign(f_b);
    if  not ( s == -1 )  
        error('Sign is equal for a = %g ( f(a)=%g ) and b = %g ( f(b)=%g ) ',  a, f_a, b, f_b);
    end
    
 %   fprintf('Root finding (Bisection method)\n');
 %   fprintf('  k\t a \t b \t c \t b-c \t f(c)\n');
    k = 1;
    while 1  
        c = a + (b-a)/2;

        f_b = feval(func, func_params, b);
        f_c = feval(func, func_params, c);
  %      fprintf('  %d\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\n',...
  %          k, a, b, c, b-c, f_c)
        
         if b - c <= epsilon
            root = c;
            break
         end
         
         if sign(f_b) * sign(f_c) <= 0
             a = c;
         else
             b = c;
         end
         
         k = k+1;
    end
    
