function ld = ray_tracing(pose, fov, nrays, countour, countour_params, precision)

	debug = true;
	
	t = pose(1:2);
	ld.nrays = nrays;
	for i=1:nrays
		if(debug)
			fprintf('@');
			if 0 == mod(i,32)
				fprintf(' %d \n', i);
			end
		end
		
		theta = -fov/2 + fov * (i-1)/(nrays-1);
		ld.theta(i) = theta;
		params = {t, pose(3)+ld.theta(i), countour, countour_params};
			
		% cerchiamo approssimativamente quello che gli si avvicina di più
		% e sarà il nostro a di partenza
		delta_a = 0.05;
		test_a = 0.001:delta_a:0.99;
		
		for j =1:size(test_a,2)
		    test_a_theta(j) = feval('eval_delta', params, test_a(j));
		end
		
		[Y,index] = min(abs(test_a_theta));
		best_a = test_a(index(1));
		best_a_val = test_a_theta(index(1));
		
		% una volta trovato a cerchiamo un b

			good_b = best_a;
			while true
				good_b = good_b - sign(best_a_val)*delta_a;
				val = feval('eval_delta', params, good_b);
				if sign(val) * sign(best_a_val) < 0
					break
				end
			end
		
		if best_a > good_b
			a = good_b;
			b = best_a;
		else
			b = good_b;
			a = best_a;
		end
		
		% do the rest with bisection
		tau = rfBisection( 'eval_delta', params, a, b,precision);
		[point, alpha] = feval(countour, countour_params, tau);
		ld.readings(i) = norm(point-t);
		%fprintf('i=%d theta=%d° reading=%f\n', i, rad2deg(ld.theta(i)), ld.readings(i));

		if cos(alpha - (pose(3)+ld.theta(i)) ) > 0
			alpha = alpha + pi;
		end

		ld.true_alpha(i) = alpha-pose(3); % local coordinates
		ld.true_alpha_abs(i) = alpha; % local coordinates
		
		if rand>0.95
			pause(0.02)
		end
	end
	if debug
		fprintf('\n');
	end
	
	ld.odometry = pose;
	ld.estimate = pose;
	ld.timestamp1 = '0';
	ld.timestamp2 = '0';
	ld.hostname = 'matlab';
	ld.points = [ cos(ld.theta) .* ld.readings; sin(ld.theta).* ld.readings];
	
function delta = eval_delta(params, tau)
	
	% secondo parametro: t
	t = params{1};
	theta = params{2};
	curve = params{3};
	curve_params = params{4};
	point = feval(curve, curve_params, tau);
	phi = angle(point-t);
	delta = normAngle(phi-theta);

%	fprintf('tau = %f  point = %f %f theta=%f phi=%d delta=%f\n', ...
%		tau, point(1), point(2), theta, phi, delta);
	
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
    
