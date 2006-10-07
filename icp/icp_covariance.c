#include <gpc_utils.h>

void compute_C_k(const gsl_vector*p_j1, const gsl_vector*p_j2, gsl_matrix*C_k)  {
	double alpha = M_PI/2 + atan2( gvg(p_j1,1)-gvg(p_j2,1), gvg(p_j1,0)-gvg(p_j2,0));
	
	gms(C_k,0,0, cos(alpha)*cos(alpha));
	gms(C_k,1,0, sin(alpha)*cos(alpha));
	gms(C_k,0,1, cos(alpha)*sin(alpha));
	gms(C_k,1,1, sin(alpha)*sin(alpha));
}

// Computes a' * B * c where B is symmetric
double quad(const gsl_matrix*a, const gsl_matrix*B, const gsl_matrix*c) {
	return gmg(a,0,0)*gmg(c,0,0)*gmg(B,0,0) + gmg(a,1,0)*gmg(c,1,0)*gmg(B,1,1)  +
	 	 2*gmg(a,1,0)*gmg(c,0,1)*gmg(B,1,0);
}

void set_polar(gsl_matrix*v,double theta,double rho) {
	gms(v, 0,0, cos(theta)*rho);
	gms(v, 1,0, sin(theta)*rho);
}

void compute_covariance_exact(LDP laser_ref, LDP laser_sens, int*valid, gsl_vector*x){
	MAT(d2J_dxdy1,3,laser_ref->nrays);
	MAT(d2J_dxdy2,3,laser_sens->nrays);
	
	// the three pieces of d2J_dx2
	MAT(d2J_dt2,2,2);
	MAT(d2J_dt_dtheta,2,1);
	MAT(d2J_dtheta2,1,1);
	
	MAT(Ck,2,2);	
	MAT(v1,2,1); MAT(v1,1,2);
	MAT(v2,2,1); MAT(v2,1,2);
	MAT(v_i,2,1);
	MAT(v3,2,1); MAT(v3t,1,2);
	MAT(v4,2,1); MAT(v4t,1,2);
	
	int i; 
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i)) continue;

		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;

		gsl_vector*p_i  = laser_sens->p[i];
		gsl_vector*p_j1  = laser_ref->p[j1];
		
		gsl_vector *p_j2  = laser_ref->p[j2];
		
		// v1 := rot(theta+PI/2)*p_i
		set_polar(v1, theta+PI/2 + laser_sens->theta[i], laser_sens->readings[i] );
		// v3 := rot(theta)*v_i)
		// v4 := rot(theta+PI/2)*v_i;
		set_polar(v3, theta + laser_sens->theta[i], 1 );
		set_polar(v4, theta + PI/2 + laser_sens->theta[i], 1 );

		m_trans(v1,v1t); m_trans(v2,v2t); m_trans(v3,v3t); m_trans(v4,v4t);
		
		v2 := (rot(theta)*p_i+t-p_j1)

		compute_C_k(q_k,other,C_k);
		
		d2J_dt2       += 2 * C_k
		d2J_dt_dtheta += 2 * v1t*C_k
		d2J_dtheta2   += 2 * quad(v2,C_k,v1) + 2 * quad(v1,C_k,v1);
		
		// for measurement rho_i  in the second scan
		
		d2Jk_dtdrho_i = 2 * v3t * C_k
		d2Jk_dtheta_drho_i = 2*quad(v2,C_k,v4)+ 2 *quad(v3,C_k,v1)
		
		d2J_dxdy2.col(c.i)[0] += d2Jk_dtdrho_i[0]
		d2J_dxdy2.col(c.i)[1] += d2Jk_dtdrho_i[1]
		d2J_dxdy2.col(c.i)[2] += d2Jk_dtheta_drho_i
	
		// for measurements rho_j1, rho_j2 in the first scan
		MAT(dC_drho_j1,2,2);
		MAT(dC_drho_j2,2,2);
		 
		dC_drho_j12(laser_ref, laser_sens, c)
		
		v_j1  := = laser_ref.points[c.j1].v
		v_j2  := = laser_ref.points[c.j2].v
		v_j1t := 
		v_j2t :=
		
		d2Jk_dtheta_drho_j1 = -2*quad(v_j1,C_k,v1) + quad(v2,dC_drho_j1,v1);
		
		d2Jk_dt_drho_j1 = 2 * (-v_j1t*m+v2t*dC_drho_j1)
		
		d2J_dxdy1.col(c.j1)[0] += d2Jk_dt_drho_j1[0]
		d2J_dxdy1.col(c.j1)[1] += d2Jk_dt_drho_j1[1]
		d2J_dxdy1.col(c.j1)[2] += d2Jk_dtheta_drho_j1
		
		# for measurement rho_j2
		d2Jk_dtheta_drho_j2 = 2 * quad(v2, dC_drho_j2, v1);
			
		d2Jk_dt_drho_j2 = 2*v2t * dC_drho_j2 
		
		d2J_dxdy1.col(c.j2)[0] += d2Jk_dt_drho_j2[0]
		d2J_dxdy1.col(c.j2)[1] += d2Jk_dt_drho_j2[1]
		d2J_dxdy1.col(c.j2)[2] += d2Jk_dtheta_drho_j2
		
	}
	
}

def compute_covariance_exact(laser_ref, laser_sens, correspondences, x)
	y1 = Vector.alloc(laser_ref.points.map{|p| p.reading}).col
	y2 = Vector.alloc(laser_sens.points.map{|p| p.reading}).col

	d2J_dxdy2 = Matrix.alloc(3, laser_sens.nrays)
	d2J_dxdy1 = Matrix.alloc(3, laser_ref.nrays)
	
	t = Vector.alloc(x[0],x[1]).col
	theta = x[2].to_f;
	
	# the three pieces of d2J_dx2
	d2J_dt2       = Matrix.alloc(2,2)
	d2J_dt_dtheta = Vector.alloc(0,0).col
	d2J_dtheta2   = 0
	
	for c in correspondences.compact
		p_k  = laser_sens.points[c.i ].cartesian
		q_k  = laser_ref .points[c.j1].cartesian

		other  = laser_ref .points[c.j2].cartesian
		v_alpha = rot(PI/2) * (q_k-other)
		v_alpha = v_alpha / v_alpha.nrm2
		m = v_alpha*v_alpha.trans
		
		
		d2J_dt2       += 2 * m
		d2J_dt_dtheta += 2 * (rot(theta+PI/2)*p_k).trans * m
		d2J_dtheta2   += 2 * (rot(theta)*p_k+t-q_k).trans * 
		   m * rot(theta+PI/2) * p_k + 2 * (rot(theta+PI/2)*p_k).trans * m *
			rot(theta+PI/2) * p_k
			
		###########
		
			
		# for measurement rho_i  in the second scan
		v_i = laser_sens.points[c.i].v
		d2Jk_dtdrho_i = 2 * (rot(theta)*v_i).trans * m
		d2Jk_dtheta_drho_i = 2*(rot(theta)*p_k+t-q_k).trans*m*rot(theta+PI/2)*v_i +
			2 *(rot(theta)*v_i).trans*m*rot(theta+PI/2)*p_k
		
		d2J_dxdy2.col(c.i)[0] += d2Jk_dtdrho_i[0]
		d2J_dxdy2.col(c.i)[1] += d2Jk_dtdrho_i[1]
		d2J_dxdy2.col(c.i)[2] += d2Jk_dtheta_drho_i
	
		# for measurements rho_j1, rho_j2 in the first scan
		dC_drho_j1, dC_drho_j2 = dC_drho_j12(laser_ref, laser_sens, c)
		
		v_j1 = laser_ref.points[c.j1].v
		v_j2 = laser_ref.points[c.j2].v
		
		d2Jk_dtheta_drho_j1 = 2 * ( -v_j1.trans*m+(rot(theta)*p_k+t-q_k).trans*dC_drho_j1)*
			rot(theta+PI/2)*p_k
		d2Jk_dt_drho_j1 = 2 * (-v_j1.trans*m+(rot(theta)*p_k+t-q_k).trans*dC_drho_j1)
		
		d2J_dxdy1.col(c.j1)[0] += d2Jk_dt_drho_j1[0]
		d2J_dxdy1.col(c.j1)[1] += d2Jk_dt_drho_j1[1]
		d2J_dxdy1.col(c.j1)[2] += d2Jk_dtheta_drho_j1
		
		# for measurement rho_j2
		d2Jk_dtheta_drho_j2 = 2*(rot(theta)*p_k+t-q_k).trans * dC_drho_j2 *
			rot(theta+PI/2)*p_k;
			
		d2Jk_dt_drho_j2 = 2*(rot(theta)*p_k+t-q_k).trans * dC_drho_j2 
		
		d2J_dxdy1.col(c.j2)[0] += d2Jk_dt_drho_j2[0]
		d2J_dxdy1.col(c.j2)[1] += d2Jk_dt_drho_j2[1]
		d2J_dxdy1.col(c.j2)[2] += d2Jk_dtheta_drho_j2
		
	end
	# put the pieces together
	d2J_dx2 = Matrix.alloc(3,3)
	d2J_dx2[0,0]=d2J_dt2[0,0]
	d2J_dx2[1,0]=d2J_dt2[1,0]
	d2J_dx2[1,1]=d2J_dt2[1,1]
	d2J_dx2[0,1]=d2J_dt2[0,1]
	d2J_dx2[2,0]=d2J_dx2[0,2]=d2J_dt_dtheta[0]
	d2J_dx2[2,1]=d2J_dx2[1,2]=d2J_dt_dtheta[1]
	d2J_dx2[2,2] = d2J_dtheta2

	dx_dy1 =  -d2J_dx2.inv * d2J_dxdy1
	dx_dy2 =  -d2J_dx2.inv * d2J_dxdy2
	
	return dx_dy1, dx_dy2
end

def getC(rho_j1,v_j1,rho_j2,v_j2)
	p_j1 = v_j1 * rho_j1
	p_j2 = v_j2 * rho_j2
	v_alpha = rot(PI/2) * (p_j1-p_j2)
	v_alpha = v_alpha / v_alpha.nrm2
	m = v_alpha*(v_alpha.trans)
	m
end

def dC_drho_j12(laser_ref, laser_sens, c)
	
	rho_j1 = laser_ref.points[c.j1].reading
	  v_j1 = laser_ref.points[c.j1].v
	rho_j2 = laser_ref.points[c.j2].reading
	  v_j2 = laser_ref.points[c.j2].v

	eps = 0.001;
	
	dC_drho_j1 = 
	  (getC(rho_j1+eps,v_j1,rho_j2,v_j2)-
		getC(rho_j1    ,v_j1,rho_j2,v_j2))/eps;

	dC_drho_j2 = 
	  (getC(rho_j1,v_j1,rho_j2+eps,v_j2)-
		getC(rho_j1,v_j1,rho_j2    ,v_j2))/eps;

	return dC_drho_j1, dC_drho_j2
	
end


