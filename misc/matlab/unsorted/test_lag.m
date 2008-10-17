function test_lag

	t_true = randn(2,1)*4;
	theta_true = deg2rad(rand*90-45);
	theta_true = deg2rad(170);
	pose_true = [t_true; theta_true];
	
	x_true = [t_true; cos(theta_true); sin(theta_true)];
	
	N=400
	points1 = rand(2,N);
	%points1 = [-1 -1; -1 1; 1 1; 1 -1]';
	points2 = rot(theta_true) * points1 + repmat(t_true,1,N) ; 
	points2 = points2 + 0.01*randn(2,N);

	f=figure;	
	plot_lambda(points1,points2,-70:0.1:20);
	
	
	f = figure;
	hold on
	plotVectors([0;0;0], points1, 'k.');
	plotVectors([0;0;0], points2, 'g.');
	axis('equal');

	tic
	[pose, L, Y] = exact_minimization_cf(points1,points2);
	toc
	
	e_t = pose(1:2)-t_true
	e_th = rad2deg(pose(3)-theta_true)
	
	plotVectors(pose, points1, 'r.');
	hold off;
	
function plot_lambda(points1,points2,interval)
	[L,Y] = create_system(points1, points2);
	ls = interval;
	size(ls,2)
	for i=1:size(ls,2)
		fu(i) = constraint({L'*L,L'*Y}, ls(i));
	end
	
	pos_index = find(fu>0);
	neg_index = find(fu<0);
	
	f = figure;
	hold on
	plot(ls(pos_index), fu(pos_index), 'b.');
	plot(ls(neg_index),fu(neg_index),'r.');
	hold off;
	

function pose = exact_minimization(points1, points2)
	[L,Y] = create_system(points1,points2);	
	
	M = diag([0 0 1 1]);
	A = (L'*L);
	B = L'*Y;
	lA = A(1:2,1:2);
	lB = A(1:2,3:4);
	lC = A(3:4,3:4);
	
	S = lC - lB' * inv(lA) * lB
	e = eig(S);
	start = - e(1)
	big_interval = 20;
	small_interval = 0.01;
	
	lambda_right = rfBisection('constraint', {L'*L,L'*Y}, ...
		start+small_interval, start+big_interval, 0.00000001)
	lambda_left = rfBisection('constraint', {L'*L,L'*Y}, ...
		start-big_interval, start-small_interval, 0.00000001)
		
	L2 = (L'*L + lambda_right * M);
	Y2 = L'*Y;
	x_hat = inv(L2) * Y2;
	
	theta_hat = atan2(x_hat(4),x_hat(3));
	pose = [x_hat(1); x_hat(2); theta_hat];
	

function [L,Y] = create_system(points1, points2)
	N = size(points1,2);
	L = zeros(2*N,4);
	Y = zeros(2*N,1);
	for i=1:N
		for j=max(1,i):min(i,N)
		p_i = points1(:,i);
		p_j = points2(:,j);
		Lk = [ eye(2) p_i rot(pi/2)*p_i ];
		Yk = p_j;
	
		L((i-1)*2+1:(i-1)*2+2,1:4) = Lk;
		Y((i-1)*2+1:(i-1)*2+2,1)   = Yk;
		end
	end
	
function [pose, L, Y] = exact_minimization_cf(points1, points2)
	[L,Y] = create_system(points1,points2);
	
	B = L'*Y;
	x1 = B(1:2); 
	x2 = B(3:4);

	A = (L'*L);
	lB = A(1:2,3:4);
	bt=lB'*lB;
	b=bt(1,1);
	
	% closed form solution
	n = A(1,1);
	c = A(3,3);
	
	pol = [ (b* x1'*x1+n^2 * x2'*x2 -2*n*x1'*lB*x2) 0 (-1)];
	r = roots(pol);
	la1 = (1/n) * (1/r(1) -n*c+b);
	la2 = (1/n) * (1/r(2) -n*c+b);
	
	M = diag([0 0 1 1]);
	L2 = (L'*L + la1 * M);
	Y2 = L'*Y;
	x_hat = inv(L2) * Y2;
	
	theta_hat = atan2(x_hat(4),x_hat(3));
	pose = [x_hat(1); x_hat(2); theta_hat];
	
	
function m = constraint(params, la)
	A = params{1};
	B = params{2};
	M = diag([0 0 1 1]);
	m = B' * inv(A+la*M)'*M*inv(A+la*M)*B - 1;
	
function root = rfBisection( func, func_params, a, b, epsilon)

    f_a = feval(func, func_params, a);
    f_b = feval(func, func_params, b);
    
    s = sign(f_a) * sign(f_b);
    if  not ( s == -1 )  
        error('Sign is equal for a = %g ( f(a)=%g ) and b = %g ( f(b)=%g ) ',  a, f_a, b, f_b);
    end
    
    fprintf('Root finding (Bisection method)\n');
   fprintf('  k\t a \t b \t c \t b-c \t f(c)\n');
    k = 1;
    while 1  
        c = a + (b-a)/2;

        f_b = feval(func, func_params, b);
        f_c = feval(func, func_params, c);
        fprintf('  %d\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\n', k, a, b, c, b-c, f_c)
        
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
    
