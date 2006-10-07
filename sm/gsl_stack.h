#include <gsl/gsl_matrix.h>

#define gmg gsl_matrix_get
#define gms gsl_matrix_set

void gsls_set(const gsl_matrix*r);
void gsls_trans();
void gsls_mult(const gsl_matrix*); 
void gsls_mult_t(const gsl_matrix*); 
void gsls_mult_left(const gsl_matrix*);
gsl_matrix* gsls_copy();
void gsls_inv(); 

double gsls_get_at(size_t  i, size_t  j);

