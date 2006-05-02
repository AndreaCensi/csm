function [p, alpha] = countour_straight(params, tau)

points = params{1};
	K = size(points,2);

	if K < 2
		error('2 points needed');
	end 	
	
	
	if tau>1
		while tau>1
			tau = tau -1;
		end
	end
	
	if tau<0
		while tau<0
			tau = tau +1;
		end
	end
	
	if tau==0
		p = points(:,1);
		return
	end
	
	if tau==1
		p = points(:,K);
		return;
	end

	
	index = ceil(tau * (K-1));
	
	
	index_tau = (index-1.0) / (K-1);
	tau2 = (tau-index_tau) * (K-1);
	
	p1 = points(:,index);
	p2 = points(:,index+1);
	
	p = p2 * tau2 + (1-tau2) * p1;
	alpha = pi/2 + atan2( p1(2)-p2(2), p1(1)-p2(1));
	
