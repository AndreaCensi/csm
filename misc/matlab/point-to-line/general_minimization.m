
function res = general_minimization(corr)
	% Input is:
	%   corr{k}.C
	%   corr{k}.p
	%   corr{k}.q
	

	%% First we put the problem in a quadratic+constraint form.
	M = zeros(4,4);
	g = zeros(4,1);
	
	for k=1:size(corr,2)
		M_k = [eye(2) [corr{k}.p (rot(pi/2)*corr{k}.p)]];
		M = M + M_k'* corr{k}.C *M_k;
		g = g + (- 2 * corr{k}.q' * corr{k}.C * M_k)';
	end
	
	W = [ zeros(2,2) zeros(2,2); zeros(2,2) eye(2)];
	
	%% This is the function that we want to minimize 
	h = @(l) g' * inv(M+2*l*W) * W * inv(M+2*l*W)' * g - 1;
	
	%% Partition M in 4 submatrixes:
	
	M = 2*M;
	% M = [A B; C D]
	A = M(1:2,1:2)
	B = M(1:2,3:4)
	D = M(3:4,3:4)
	
	S = D - B' * inv(A) * B;
	Sa = inv(S) * det(S);
		
	g1 = g(1:2); g2=g(3:4);

   p7 = [( g1'*(inv(A)*B*  4    *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B*  4   )*g2  + g2'*( 4   )*g2) ...
	      ( g1'*(inv(A)*B*  4*Sa *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B*  4*Sa)*g2  + g2'*( 4*Sa)*g2) ...
			( g1'*(inv(A)*B* Sa*Sa *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B* Sa*Sa)*g2  + g2'*(Sa*Sa)*g2)];
			
	p_lambda = [4 (2*S(1,1)+2*S(2,2)) (S(1,1)*S(2,2)-S(2,1)*S(1,2))];
	Ptot = polyadd(p7, -conv(p_lambda,p_lambda)) ;

	% Find largest real root of Ptot
	r = roots(Ptot);
	lambda = 0;
	for i=1:4
		if isreal(r(i)) & (r(i)>0)
			lambda = max(lambda, r(i));
		end
	end
	
	x = -inv(M + 2 * lambda * W) * g;
	theta = atan2(x(4),x(3));
	
	
	res.x = [x(1); x(2); theta]
	
	
	
	
%	Q = @(l) (S+2*l*eye(2))
%	h2 = @(l) g' * [ inv(A)*B*inv(Q(l))*inv(Q(l))*B'*inv(A)'  -inv(A)*B*inv(Q(l))*inv(Q(l)); ...
%	(-inv(A)*B*inv(Q(l))*inv(Q(l)))' inv(Q(l))*inv(Q(l))] * g - 1;
	
	
%	h8 = @(l) polyval(p7,l)- polyval(p_lambda,l)^2;
		
	%QQ = @(l) (Sa*Sa+4*(l^2)*eye(2)+4*l*Sa) / ( polyval(p_lambda,l)^2)
	
	
	%h3 = @(l) g' * [ inv(A)*B*QQ(l)*B'*inv(A)'  -inv(A)*B*QQ(l); ...
	 %              (-inv(A)*B*QQ(l))' QQ(l)] * g - 1;
	
%	h4 = @(l) g1'* inv(A)*B*QQ(l)*B'*inv(A)'*g1 + g1'*-inv(A)*B*QQ(l)*g2 +  ...
%	               g2' * (-inv(A)*B*QQ(l))'*g1 +g2'*QQ(l)*g2 - 1;

%	h5 = @(l) g1'* inv(A)*B*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*B'*inv(A)'*g1 + g1'*-inv(A)*B*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*g2 +  ...
%	               g2' * (-inv(A)*B*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa))'*g1 +g2'*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*g2 - polyval(p_lambda,l)^2;

%	h6 = @(l) g1'* inv(A)*B*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*B'*inv(A)*g1 ...
%	          + 2*g1'*-inv(A)*B*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*g2 +  ...
%	             +g2'*(Sa*Sa+4*(l^2)*eye(2)+4*l*Sa)*g2 - polyval(p_lambda,l)^2;
	
%	h7 = @(l) (l^2) * ( g1'*(inv(A)*B*  4    *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B*  4   )*g2  + g2'*( 4   )*g2) + ...
%	          (l)   * ( g1'*(inv(A)*B*  4*Sa *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B*  4*Sa)*g2  + g2'*( 4*Sa)*g2) + ...
%	                  ( g1'*(inv(A)*B* Sa*Sa *B'*inv(A))*g1 + 2*g1'*(-inv(A)*B* Sa*Sa)*g2  + g2'*(Sa*Sa)*g2) - polyval(p_lambda,l)^2;

