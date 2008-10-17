function ld = computeSurfaceNormals(ld, maxDist)

n = size(ld.points,2);

for i=1:n

	% consider all points in a ball of radius maxDist 
	imin=i; stop=0;
	while stop == 0 & imin>1 
		if norm(ld.points(:,i)-ld.points(:,imin-1)) < maxDist
			imin = imin -1;
		else
			stop=1;
		end
	end
	
	imax=i; stop=0;
	while stop==0 & imax<n
		if norm(ld.points(:,i)-ld.points(:,imax+1)) < maxDist
			imax=imax+1;
		else 
			stop = 1;
		end
	end

	num=imax-imin+1;

	if num>1 
		%[theta; rho; error] 
		[alpha,rho,error] = regression(ld.points(:,imin:imax));
	
		ld.alpha(i) =  normalize(alpha);
		ld.alpha_valid(i) = 1;
		ld.alpha_error(i) = sqrt(error/num);
		
		if num==2
			ld.alpha_error(i) = 0.1;
		end
		
		%fprintf('n = %d alpha %3.3f rho %5.5f error %f\n', ...
		%imax-imin+1, alpha,rho,ld.alpha_error(i)); 
	else
		fprintf('ops! %d\n',i);
		ld.alpha(i) = nan;
		ld.alpha_valid(i) = 0;
		ld.alpha_error(i) = nan;
	end
	
end

function res = normalize(alpha)
	while alpha > 2*pi
		alpha = alpha - 2*pi;
	end
	while alpha < 0
		alpha = alpha + 2*pi;
	end
	res = alpha;
	
