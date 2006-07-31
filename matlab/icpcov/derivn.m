%% Numerical derivation with step \verb|epsilon|
function res = deriv(fh, x, epsilon)
	% deriv(fh, x, epsilon)
	%  fh: function handle
	%   x: point to derive
	% epsilon: interval
	f1 = fh(x+epsilon/2);
	f0 = fh(x-epsilon/2);
	res= (f1-f0)/epsilon;

