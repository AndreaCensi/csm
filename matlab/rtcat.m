
function d = rtcat(ref, delta)
	d(1:2) = ref(1:2) + rot(ref(3)) * delta(1:2); 
	d(3) = ref(3) + delta(3);
	d = d';

