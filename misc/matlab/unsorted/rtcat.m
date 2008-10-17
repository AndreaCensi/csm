function d = rtcat(ref, delta)
	%% Adds a rototranslation. ref is the starting point and
	%% delta is (x,y,theta). 
	d(1:2) = ref(1:2) + rot(ref(3)) * delta(1:2); 
	d(3) = ref(3) + delta(3);
	d = d';

