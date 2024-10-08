/**************************************************************************
**
** Copyright (C) 1993 David E. Steward & Zbigniew Leyk, all rights reserved.
**
**			     Meschach Library
** 
** This Meschach Library is provided "as is" without any express 
** or implied warranty of any kind with respect to this software. 
** In particular the authors shall not be liable for any direct, 
** indirect, special, incidental or consequential damages arising 
** in any way from use of the software.
** 
** Everyone is granted permission to copy, modify and redistribute this
** Meschach Library, provided:
**  1.  All copies contain this copyright notice.
**  2.  All modified copies shall carry a notice stating who
**      made the last modification and the date of such modification.
**  3.  No charge is made for this software or works derived from it.  
**      This clause shall not be construed as constraining other software
**      distributed on the same medium as this software, nor is a
**      distribution fee considered a charge.
**
***************************************************************************/
/*
	A collection of functions for computing norms: scaled and unscaled
*/
static char rcsid[] = "$Id: norm.c,v 1.6 1994/01/13 05:34:35 des Exp $";
#include	<stdio.h>
#include	<math.h>
#include	"matrix.h"
/* _v_norm1 -- computes (scaled) 1-norms of vectors */
#ifndef ANSI_C
#else

double _v_norm1(const VEC *x,const VEC *scale)
#endif
{
  int i;
  int dim;
  double s;
  double sum;
  if (x == ((VEC *)((void *)0))) 
    ev_err("norm.c",8,49,"_v_norm1",0);
  dim = (x -> dim);
  sum = 0.0;
  if (scale == ((VEC *)((void *)0))) 
    for (i = 0; i <= dim - 1; i += 1) {
      sum += fabs(x -> ve[i]);
    }
   else if (scale -> dim < dim) 
    ev_err("norm.c",1,57,"_v_norm1",0);
   else 
    for (i = 0; i <= dim - 1; i += 1) {
      s = scale -> ve[i];
      sum += (s == 0.0?fabs(x -> ve[i]) : fabs(x -> ve[i] / s));
    }
  return sum;
}
/* square -- returns x^2 */
#ifndef ANSI_C
#else

double square(double x)
#endif
{
  return x * x;
}
/* cube -- returns x^3 */
#ifndef ANSI_C
#else

double cube(double x)
#endif
{
  return x * x * x;
}
/* _v_norm2 -- computes (scaled) 2-norm (Euclidean norm) of vectors */
#ifndef ANSI_C
#else

double _v_norm2(const VEC *x,const VEC *scale)
#endif
{
  int i;
  int dim;
  double s;
  double sum;
  if (x == ((VEC *)((void *)0))) 
    ev_err("norm.c",8,97,"_v_norm2",0);
  dim = (x -> dim);
  sum = 0.0;
  if (scale == ((VEC *)((void *)0))) 
    for (i = 0; i <= dim - 1; i += 1) {
      sum += square(x -> ve[i]);
    }
   else if (scale -> dim < dim) 
    ev_err("norm.c",1,105,"_v_norm2",0);
   else 
    for (i = 0; i <= dim - 1; i += 1) {
      s = scale -> ve[i];
      sum += (s == 0.0?square(x -> ve[i]) : square(x -> ve[i] / s));
    }
  return sqrt(sum);
}
#define	max(a,b)	((a) > (b) ? (a) : (b))
/* _v_norm_inf -- computes (scaled) infinity-norm (supremum norm) of vectors */
#ifndef ANSI_C
#else

double _v_norm_inf(const VEC *x,const VEC *scale)
#endif
{
  int i;
  int dim;
  double s;
  double maxval;
  double tmp;
  if (x == ((VEC *)((void *)0))) 
    ev_err("norm.c",8,130,"_v_norm_inf",0);
  dim = (x -> dim);
  maxval = 0.0;
  if (scale == ((VEC *)((void *)0))) 
    for (i = 0; i <= dim - 1; i += 1) {
      tmp = fabs(x -> ve[i]);
      maxval = (maxval > tmp?maxval : tmp);
    }
   else if (scale -> dim < dim) 
    ev_err("norm.c",1,140,"_v_norm_inf",0);
   else 
    for (i = 0; i <= dim - 1; i += 1) {
      s = scale -> ve[i];
      tmp = (s == 0.0?fabs(x -> ve[i]) : fabs(x -> ve[i] / s));
      maxval = (maxval > tmp?maxval : tmp);
    }
  return maxval;
}
/* m_norm1 -- compute matrix 1-norm -- unscaled */
#ifndef ANSI_C
#else

double m_norm1(const MAT *A)
#endif
{
  int i;
  int j;
  int m;
  int n;
  double maxval;
  double sum;
  if (A == ((MAT *)((void *)0))) 
    ev_err("norm.c",8,163,"m_norm1",0);
  m = (A -> m);
  n = (A -> n);
  maxval = 0.0;
  for (j = 0; j <= n - 1; j += 1) {
    sum = 0.0;
    for (i = 0; i <= m - 1; i += 1) {
      sum += fabs(A -> me[i][j]);
    }
    maxval = (maxval > sum?maxval : sum);
  }
  return maxval;
}
/* m_norm_inf -- compute matrix infinity-norm -- unscaled */
#ifndef ANSI_C
#else

double m_norm_inf(const MAT *A)
#endif
{
  int i;
  int j;
  int m;
  int n;
  double maxval;
  double sum;
  if (A == ((MAT *)((void *)0))) 
    ev_err("norm.c",8,191,"m_norm_inf",0);
  m = (A -> m);
  n = (A -> n);
  maxval = 0.0;
  for (i = 0; i <= m - 1; i += 1) {
    sum = 0.0;
    for (j = 0; j <= n - 1; j += 1) {
      sum += fabs(A -> me[i][j]);
    }
    maxval = (maxval > sum?maxval : sum);
  }
  return maxval;
}
/* m_norm_frob -- compute matrix frobenius-norm -- unscaled */
#ifndef ANSI_C
#else

double m_norm_frob(const MAT *A)
#endif
{
  int i;
  int j;
  int m;
  int n;
  double sum;
  if (A == ((MAT *)((void *)0))) 
    ev_err("norm.c",8,219,"m_norm_frob",0);
  m = (A -> m);
  n = (A -> n);
  sum = 0.0;
  for (i = 0; i <= m - 1; i += 1) {
    for (j = 0; j <= n - 1; j += 1) {
      sum += square(A -> me[i][j]);
    }
  }
  return sqrt(sum);
}
