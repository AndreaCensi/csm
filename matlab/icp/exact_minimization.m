function [pose, L, Y] = exact_minimization(points1, points2)
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
	
