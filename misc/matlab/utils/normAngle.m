function res = normAngle(theta)

	for i=1:size(theta,2)
		while theta(i)>pi
			 theta(i) = theta(i) - 2*pi;
		end
		
		while theta(i)<-pi
			 theta(i) = theta(i) + 2*pi;
		end
	end
	
res = theta;