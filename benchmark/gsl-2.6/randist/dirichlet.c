/* randist/dirichlet.c
 * 
 * Copyright (C) 2007 Brian Gough
 * Copyright (C) 2002 Gavin E. Crooks <gec@compbio.berkeley.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>


/* The Dirichlet probability distribution of order K-1 is 

     p(\theta_1,...,\theta_K) d\theta_1 ... d\theta_K = 
        (1/Z) \prod_i=1,K \theta_i^{alpha_i - 1} \delta(1 -\sum_i=1,K \theta_i)

   The normalization factor Z can be expressed in terms of gamma functions:

      Z = {\prod_i=1,K \Gamma(\alpha_i)} / {\Gamma( \sum_i=1,K \alpha_i)}  

   The K constants, \alpha_1,...,\alpha_K, must be positive. The K parameters, 
   \theta_1,...,\theta_K are nonnegative and sum to 1.

   The random variates are generated by sampling K values from gamma
   distributions with parameters a=\alpha_i, b=1, and renormalizing. 
   See A.M. Law, W.D. Kelton, Simulation Modeling and Analysis (1991).

   Gavin E. Crooks <gec@compbio.berkeley.edu> (2002)
*/

static void ran_dirichlet_small (const gsl_rng * r, const size_t K, const double alpha[], double theta[]);

void
gsl_ran_dirichlet (const gsl_rng * r, const size_t K,
                   const double alpha[], double theta[])
{
  size_t i;
  double norm = 0.0;

  for (i = 0; i < K; i++)
    {
      theta[i] = gsl_ran_gamma (r, alpha[i], 1.0);
    }
  
  for (i = 0; i < K; i++)
    {
      norm += theta[i];
    }

  if (norm < GSL_SQRT_DBL_MIN)  /* Handle underflow */
    {
      ran_dirichlet_small (r, K, alpha, theta);
      return;
    }

  for (i = 0; i < K; i++)
    {
      theta[i] /= norm;
    }
}


/* When the values of alpha[] are small, scale the variates to avoid
   underflow so that the result is not 0/0.  Note that the Dirichlet
   distribution is defined by a ratio of gamma functions so we can
   take out an arbitrary factor to keep the values in the range of
   double precision. */

static void 
ran_dirichlet_small (const gsl_rng * r, const size_t K,
                     const double alpha[], double theta[])
{
  size_t i;
  double norm = 0.0, umax = 0;

  for (i = 0; i < K; i++)
    {
      double u = log(gsl_rng_uniform_pos (r)) / alpha[i];
      
      theta[i] = u;

      if (u > umax || i == 0) {
        umax = u;
      }
    }
  
  for (i = 0; i < K; i++)
    {
      theta[i] = exp(theta[i] - umax);
    }
  
  for (i = 0; i < K; i++)
    {
      theta[i] = theta[i] * gsl_ran_gamma (r, alpha[i] + 1.0, 1.0);
    }

  for (i = 0; i < K; i++)
    {
      norm += theta[i];
    }

  for (i = 0; i < K; i++)
    {
      theta[i] /= norm;
    }
}





double
gsl_ran_dirichlet_pdf (const size_t K,
                       const double alpha[], const double theta[])
{
  return exp (gsl_ran_dirichlet_lnpdf (K, alpha, theta));
}

double
gsl_ran_dirichlet_lnpdf (const size_t K,
                         const double alpha[], const double theta[])
{
  /*We calculate the log of the pdf to minimize the possibility of overflow */
  size_t i;
  double log_p = 0.0;
  double sum_alpha = 0.0;

  for (i = 0; i < K; i++)
    {
      log_p += (alpha[i] - 1.0) * log (theta[i]);
    }

  for (i = 0; i < K; i++)
    {
      sum_alpha += alpha[i];
    }

  log_p += gsl_sf_lngamma (sum_alpha);

  for (i = 0; i < K; i++)
    {
      log_p -= gsl_sf_lngamma (alpha[i]);
    }

  return log_p;
}