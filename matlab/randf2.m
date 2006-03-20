function [f0,f1,f2] = randf2(t,w)

	n = size(t,2);
	f2 = w * randn(1,n+2);
	
	f1(1)=0;
	for i=2:n
		f1(i) = f1(i-1) + f2(i-1) * (t(i)-t(i-1));
	end

	f0(1)=0;
	for i=2:n
		f0(i) = f0(i-1) + f1(i-1) * (t(i)-t(i-1));
	end
	
	f2 = f2(1:n);
	f1 = f1(1:n);
	f0 = f0(1:n);
	
	
	
	figure 
	hold on
	
	plot(1:n, f0, 'r.');
	plot(1:n, f1, 'g.');
	plot(1:n, f2, 'b.');
	
	hold off
