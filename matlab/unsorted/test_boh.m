function f = test_boh

th=0:0.001:2*pi;

G = [.5 4; 0 .5]; rand(2,2);
k = 100*rand(2,1);
k =[-1;0];

for i=1:size(th,2)

	f(i) = vers(th(i))' * G * vers(th(i)) + k'*vers(th(i));

end

fi=figure;
plot(rad2deg(th),f);



function v = vers(t)
	v=[cos(t);sin(t)];
