#include <math.h>
#include "laser_data.h"
#include <gpc_utils.h>
#include "easy_gsl.h"
#include "easy_gsl_macros.h"

val compute_C_k(val p_j1, val p_j2)  {	
	val d = sub(p_j1, p_j2);
	double alpha = M_PI/2 + atan2( atv(d,1), atv(d,0));
	double c = cos(alpha); double s = sin(alpha);
	double m[2][2] = {
		{c*c, c*s},
		{c*s, s*s}
	};
	return egsl_vFda(2,2,m);
}



val dC_drho(val p1, val p2) {
	double eps = 0.001;

	val C_k = compute_C_k(p1, p2);	
	val p1b = sum(p1, sc(eps/egsl_norm(p1),p1));
	val C_k_eps = compute_C_k(p1b,p2);
	return sc(1/eps, sub(C_k, C_k_eps));
}


void compute_covariance_exact(LDP laser_ref, LDP laser_sens, gsl_vector*x) {
	egsl_push();
	
	val d2J_dxdy1 = zeros(3,laser_ref->nrays);
	val d2J_dxdy2 = zeros(3,laser_sens->nrays);

	// the three pieces of d2J_dx2
	val d2J_dt2       = zeros(2,2);
	val d2J_dt_dtheta = zeros(2,1);
	val d2J_dtheta2   = zeros(1,1);
	
	double theta = x->data[2];
	val t = egsl_vFa(2,x->data);
	
	int i; 
	for(i=0;i<laser_sens->nrays;i++) {
		if(!ld_valid_corr(laser_sens,i)) continue;
		egsl_push();

		int j1 = laser_sens->corr[i].j1;
		int j2 = laser_sens->corr[i].j2;

		val p_i  = egsl_vFgslv(laser_sens->p[i]);
		val p_j1 = egsl_vFgslv(laser_ref ->p[j1]);
		val p_j2 = egsl_vFgslv(laser_ref ->p[j2]);
		
		// v1 := rot(theta+PI/2)*p_i
		val v1 = m(rot(theta+M_PI/2), p_i);		
		// v2 := (rot(theta)*p_i+t-p_j1)
		val v2 = sum3( m(rot(theta),p_i), t, minus(p_j1));
		// v3 := rot(theta)*v_i
		val v3 = vers(theta + laser_sens->theta[i]);
		// v4 := rot(theta+PI/2)*v_i;
		val v4 = vers(theta + laser_sens->theta[i] + M_PI/2);
		
		val C_k = compute_C_k(p_j1, p_j2);
		add_to(d2J_dt2, sc(2.0, C_k));
		add_to(d2J_dt_dtheta, sc(2.0,m(tr(v1),C_k)));
		add_to(d2J_dtheta2, sc(2.0, sum( m3(tr(v2),C_k,v1), m3(tr(v1),C_k,v1))));
		
		// for measurement rho_i  in the second scan
		val d2Jk_dtdrho_i = sc(2.0, m(tr(v3), C_k));
		val d2Jk_dtheta_drho_i = sc(2.0, sum( m3(tr(v2),C_k,v4),  m3(tr(v3),C_k,v1)));
		add_to_col(d2J_dxdy2, i, comp_col(d2Jk_dtdrho_i, d2Jk_dtheta_drho_i));
		
		// for measurements rho_j1, rho_j2 in the first scan
		
		val dC_drho_j1 = dC_drho(p_j1, p_j2);
		val dC_drho_j2 = dC_drho(p_j2, p_j1);
		
		val v_j1 = vers(laser_ref->theta[j1]);
		val v_j2 = vers(laser_ref->theta[j2]);
		
		val d2Jk_dt_drho_j1 = sum(sc(-2.0,m(tr(v_j1),C_k)), sc(2.0,m(tr(v2),dC_drho_j1)));
		val d2Jk_dtheta_drho_j1 = sum( sc(-2.0, m3(tr(v_j1),C_k,v1)), m3(tr(v2),dC_drho_j1,v1));
		add_to_col(d2J_dxdy1, j1, comp_col(d2Jk_dt_drho_j1, d2Jk_dtheta_drho_j1));
		
		// for measurement rho_j2
		val d2Jk_dt_drho_j2 = 2 * m( tr(v2), dC_drho_j2);
		val d2Jk_dtheta_drho_j2 = 2 * m3( tr(v2), dC_drho_j2, v1);
		add_to_col(d2J_dxdy1, j2, comp_col(d2Jk_dt_drho_j2, d2Jk_dtheta_drho_j2));

		egsl_pop();
	}
	egsl_pop();	
}



