#include "Optimizer.h"

#include <gsl/gsl_linalg.h>
 #include <gsl/gsl_blas.h>


Optimizer::Optimizer(LDP ld, MeasurementsLikelihood ml, ConstraintManager cm)
{
	laser_data = ld;
	measurements_likelihood = ml;
	constraint_manager = cm;

}

Optimizer::~Optimizer(void)
{

}

std::vector<double> Optimizer::OptimizeAlphas()
{
	std::vector<double> x;
	
	std::vector<double> alpha0_vector;
	std::vector<double> covs_vector;
	for(int l=0;l<laser_data->nrays;l++)
	{
		alpha0_vector.push_back(laser_data->alpha[l]);
		x.push_back(laser_data->alpha[l]);
		covs_vector.push_back(laser_data->cov_alpha[l]);
	
	}
	
	//********************************************************************************
	if (measurements_likelihood.function_type == L2)	//L2, Newton minimization
	{
		double lambda = 0.1;
			
		for (int k=0;k<1000;k++)
		{
			// chek...
			if (lambda < 0.0000000000000001)
			{
				printf ("Lambda = %f  is too small for meaningful results, exiting.\n", lambda);
				return x;
			}
			
			constraint_manager.ApplyConstraintsAlphas(x);
			measurements_likelihood.ComputeAlphaLikelihoods(x,alpha0_vector,covs_vector);
			
			int size = measurements_likelihood.grad.size();
			double gradient[size];
			for (int i=0;i<size;i++)
			{
				gradient[i] = constraint_manager.grad[i] + lambda*measurements_likelihood.grad[i];  
			}
			
			int size_h = measurements_likelihood.hess.size();
			if (size_h != size)
				sm_error("Gradient and Hessian dimensions must agree\n");
			double hessian[size_h*size_h];
			int ne = 0;
			for(int i=0;i<size_h;i++)
				for(int j=0;j<size_h;j++)
			{
					hessian[ne] = constraint_manager.hess[i][j] + lambda*measurements_likelihood.hess[i][j];
					ne++;
			}
			
			gsl_matrix_view h = gsl_matrix_view_array(hessian,size_h,size_h);
			gsl_vector_view g = gsl_vector_view_array(gradient,size);		
			
			gsl_vector *dx = gsl_vector_alloc (4);
       
         int s;
     
         gsl_permutation * p = gsl_permutation_alloc (size_h);
     
         gsl_linalg_LU_decomp (&h.matrix, p, &s);
     
         gsl_linalg_LU_solve (&h.matrix, p, &g.vector, dx);

			for (int i=0;i<size;i++)
			{
				x[i]-= gsl_vector_get(dx,i);
			}
			
			double err_k;
			const gsl_vector* const_g = &g.vector;
			gsl_blas_ddot (const_g, dx, &err_k);
			
			
			gsl_permutation_free (p);
       	gsl_vector_free (dx);
       	
       	if (err_k < 0.000000001)
       		lambda = lambda * 0.1;

			
		}
	
	
	}
	
	//********************************************************************************
	if (measurements_likelihood.function_type == L1)		//L1, regularized problem
	{
	
	
	}

}

std::vector<double> Optimizer::OptimizeRanges()
{

}

/*std::vector<Pose> Optimizer::OptimizePoses()
{


}*/
