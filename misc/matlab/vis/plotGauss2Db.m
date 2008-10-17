% Non disegna il punto
function res = plotGauss2Db(mu,sigma,color)

if (size(mu,2) ~= 1)
    mu = mu';
    if (size(mu,2) ~= 1)
        error('mu deve essere un vettore');
    end
end
if (size(mu,2) ~= 1)
    error('mu deve essere un vettore bidimensionale');
end
if (size(sigma,1) ~= size(sigma,2))
    error('sigma deve essere quadrata');
end
if (size(sigma,1) ~= 2)
    error('la funzione plotGauss2D serve solo per plottare in 2 dimensioni');
end

res = ellipse(mu,sigma,color);

function res = ellipse(mean,Sigma,color)
	MahlDist = 6;
	npoints = 100;
	% Prende un cerchio e fa una trasformazione affine
	theta=(0:1:npoints)*(2*pi/npoints);
	[V,D] = eig(Sigma);
	points= repmat(mean,1,npoints+1) + ...
		V * sqrt(D) * sqrt(MahlDist) * [cos(theta); sin(theta)];
	res = plot(points(1,:),points(2,:),color);
