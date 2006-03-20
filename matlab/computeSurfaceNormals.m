function ld = computeSurfaceNormals(ld, maxDist)

n = size(ld.points,2);

for i=1:n

	% consider all points in a ball of radius maxDist 
	imin=i; stop=0;
	while stop == 0 & imin>1 
		if vectorNorm2(ld.points(:,i)-ld.points(:,imin-1)) < maxDist
			imin = imin -1;
		else
			stop=1;
		end
	end
	
	imax=i; stop=0;
	while stop==0 & imax<n
		if vectorNorm2(ld.points(:,i)-ld.points(:,imax+1)) < maxDist
			imax=imax+1;
		else 
			stop = 1;
		end
	end

	n=imax-imin+1;

	if n>1 
		%[theta; rho; error] 
		[alpha,rho,error] = regression(ld.points(:,imin:imax));	
	
		ld.alpha(i) =  alpha;
		ld.alpha_valid(i) = 1;
		ld.alpha_error(i) = sqrt(error/n);
	else
		ld.alpha(i) = nan;
		ld.alpha_valid(i) = 0;
		ld.alpha_error(i) = nan;
	end
	
end

