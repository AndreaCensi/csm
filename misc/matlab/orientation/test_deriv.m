function test_deriv

% punti casuali
pt=[-1 -0.3 0 0.1  0.3 1];
rand('seed',8);
st=cos(15.32*pt);
%rand(1, size(pt,2))*4;

% polinomio casuale
P = polyfit(pt,st, min(3,size(pt,2)-1) );
Pd = polyder(P);
Pdd = polyder(Pd);

t=-1:0.01:1;
f0=polyval(P,t);
fd=polyval(Pd,t);
f2=polyval(Pdd,t);

if true
	vu = 7;
	
	f0 = sin(vu*t) + 0 * sin(vu*10*t)*0.1;
	fd = vu * cos(vu*t)+ 0 * cos(vu*10*t);
	f2 = - vu * vu * sin(vu*t);
end

if false
	W = 100;
	[f0,fd,f2] = randf2(t, W);
end
if false
	f0 =  zeros(1,size(t,2));
	fd =  zeros(1,size(t,2));
	f2 =  zeros(1,size(t,2));
end

if false
	f0 = t; % zeros(1,size(t,2));
	fd = 1 * ones(1,size(t,2));
	f2 = zeros(1,size(t,2));
end

curv= sum(f2.*f2) / size(f2,2);

curv=0;
sigma=0.1; 
randn('seed',4);
y= f0 + sigma*randn(1,size(t,2));

f0cov = ones(1,size(t,2)) * sigma*sigma;
myf1guess = zeros(1,size(t,2));
interval = 4;
[est_fd, est_var] = estimate_derivative(t, y, f0cov, curv, myf1guess, interval);
[est_fd2, est_var2] = estimate_derivative(t, y, f0cov, curv, est_fd, interval);


% percentuale entro 95%
white = (est_fd-fd) ./ sqrt(est_var);

fprintf('Curv: %f\n',curv);

for p=1:3
	perc = sum( abs(white) < p ) / size(white,2);
	fprintf('Perc: %d %f\n',p,perc);
end


fprintf('mean sigma= %f \n',mean(sqrt(est_var)));

% errore

fprintf('errore quadr. medio 1: %f \n', norm(est_fd-fd));
fprintf('errore quadr. medio 2: %f \n', norm(est_fd2-fd));


f=figure;
subplot(3,1,1);
hold on;
plot(t,f0,'r.');
plot(t,y,'bd');

subplot(3,1,2);
hold on;
U = sqrt(est_var)*3;
%plot(t,est_fd,'kd');
errorbar(t,est_fd,U,U,'k');
plot(t,fd,'g.');
%plot(t,est_fd2,'b.');

subplot(3,1,3);
plot(t, white);

%plot(t,f2,'b.');
hold off;

%legend('poli', 'deriv', 'misure', 'stima deriv');
