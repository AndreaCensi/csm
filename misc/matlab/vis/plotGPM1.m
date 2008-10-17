function plotGPM1(result)
	hold on
	
	n = size(result.T, 2);

	for i=1:n	
		if result.weight(i) < 0.8
			continue
		end
		
		T = result.T(:, i);
		phi = result.phi(i);
		gamma = result.alpha(i);
		
		length = 0.5;
		p1 = T + vers(gamma-pi/2) * length;
		p2 = T + vers(gamma+pi/2) * length;
		
		plot([p1(1) p2(1)],[p1(2) p2(2)],'b-');
		%plot(T(1),T(2),'r.');
		
	end
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';
