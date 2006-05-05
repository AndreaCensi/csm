function rnorm = sample_normal(mean,Sigma,N)
% sample_normal(mean,Sigma,N)
%  sampleNormal generates a pxN array  of normal draws
%  from the p-dim N(mean,Sigma)
	p=length(mean); 
	rnorm = repmat(reshape(mean,p,1),1,N) +chol(Sigma)'*randn(p,N);

