
% params.laserData1
% params.laserData2
% params.maxAngularCorrectionDeg
% params.maxLinearCorrection
% params.emXYsigma

function res = gpm(params)

	% GHT o(n^2) algorithm
	% This loop can be much more efficient ( o(n) )
	k=1;
	for a=find(params.laserData1.alpha_valid)
		p = params.laserData1.points(:,a);
		delta = abs(deg2rad(params.maxAngularCorrectionDeg)); 
		delta = delta + abs(atan(params.maxLinearCorrection/vectorNorm2(p)));
		
		angleRes = pi / size(params.laserData2.points,2);
		range = ceil(delta/angleRes);
		from = a-range;
		to = a+range;
		from = max(from, 1);
		to   = min(to, size(params.laserData2.points,2));
			
	for b=from:to
	
		if params.laserData2.alpha_valid(b)==0
			continue;
		end
	
		phi = params.laserData1.alpha(a) - params.laserData2.alpha(b);
		phi = normAngle(phi);
		
		if abs(phi) > deg2rad(params.maxAngularCorrectionDeg)
			continue
		end
		
		T = params.laserData1.points(:,a) - rot(params.laserData2.points(:,b), phi);
		
		if vectorNorm2(T) > params.maxLinearCorrection
			continue
		end
		
		% Surface normal
		gamma = params.laserData1.alpha(a);
		
		slide=1;
		if slide 
			% Let T slide along alpha
			T2 = vers(gamma) * (vers(gamma)'*T);
		end

		sigma = diag([sq(params.emXYsigma) sq(params.emXYsigma)]);
		evModelWeight = mynormpdf(T, [0;0], sigma);
		
		weight=1;
		weight = weight *evModelWeight;
		weight = weight * params.laserData1.alpha_weight(a);
		weight = weight * params.laserData2.alpha_weight(b);
		%weight = weight * sqrt(params.laserData1.readings(a)*params.laserData2.readings(b)); 
		res.T(:,k) = T;
		res.T2(:,k) = T2;
		res.phi(k) = phi; 
		res.alpha(k) = gamma; 
		res.weight(k) = weight; 
		k=k+1;
	end
	end
	
res.weight = normalize01(res.weight) ; 

function b = normalize01(a)
	b = (a-min(a))/(max(a)-min(a));
	
function res = rot(v, phi)
	res = [cos(phi) -sin(phi); sin(phi) cos(phi)] * v;
	
function v = vers(theta)
	v = [cos(theta) sin(theta)]';

function q = sq(v)
	q=v*v;
	
function p = mynormpdf(x, mu, sigma);
    mahal = (x-mu)' * inv(sigma) * (x-mu);    
	 p = (1 / (sqrt(2*pi) * det(sigma))) * exp(-0.5*mahal);
	 
