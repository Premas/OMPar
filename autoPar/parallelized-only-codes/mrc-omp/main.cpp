#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <random>
#include <omp.h>
#include "reference.h"
#include <omp.h> 

void MRCGradient(const int N,const int *Y,const float *X1,const float *X2,const float *dOutput,const float margin,float *dX1,float *dX2)
{
  
#pragma omp parallel for firstprivate (N,margin)
  for (int i = 0; i <= N - 1; i += 1) {
    float dist = (-Y[i]) * (X1[i] - X2[i]) + margin;
    if (dist < 0.f) {
      dX1[i] = dX2[i] = 0.f;
    }
     else {
      dX1[i] = (-Y[i]) * dOutput[i];
      dX2[i] = Y[i] * dOutput[i];
    }
  }
}

void MRCGradient2(const int N,const int *Y,const float *X1,const float *X2,const float *dOutput,const float margin,float *dX1,float *dX2)
{
  
#pragma omp parallel for firstprivate (N,margin)
  for (int i = 0; i <= N - 1; i += 1) {
    float y = Y[i];
    float o = dOutput[i];
    float dist = -y * (X1[i] - X2[i]) + margin;
    dX1[i] = (dist < 0.f?0.f : -y * o);
    dX2[i] = (dist < 0.f?0.f : y * o);
  }
}

int main(int argc,char *argv[])
{
  if (argc != 3) {
    printf("Usage: %s <number of elements> <repeat>\n",argv[0]);
    return 1;
  }
  const int length = atoi(argv[1]);
  const int repeat = atoi(argv[2]);
  size_t size_bytes = length * sizeof(float );
  float *h_X1 = (float *)(malloc(size_bytes));
  float *h_X2 = (float *)(malloc(size_bytes));
  float *h_O = (float *)(malloc(size_bytes));
  int *h_Y = (int *)(malloc(size_bytes));
  float *h_dX1 = (float *)(malloc(size_bytes));
  float *h_dX2 = (float *)(malloc(size_bytes));
  float *r_dX1 = (float *)(malloc(size_bytes));
  float *r_dX2 = (float *)(malloc(size_bytes));
  const float m = 0.01;
// margin
  std::default_random_engine g(123);
  class std::uniform_real_distribution< float  > distr(- 2.f,2.f);
  for (int i = 0; i <= length - 1; i += 1) {
    h_X1[i] = distr(g);
    h_X2[i] = distr(g);
    h_O[i] = distr(g);
    h_Y[i] = (distr(g) < 0?- 1 : 1);
  }
{
// warmup
    for (int i = 0; i <= repeat - 1; i += 1) {
      MRCGradient(length,h_Y,h_X1,h_X2,h_O,m,h_dX1,h_dX2);
      MRCGradient2(length,h_Y,h_X1,h_X2,h_O,m,h_dX1,h_dX2);
    }
    auto start = std::chrono::_V2::steady_clock::now();
    for (int i = 0; i <= repeat - 1; i += 1) {
      MRCGradient(length,h_Y,h_X1,h_X2,h_O,m,h_dX1,h_dX2);
    }
    auto end = std::chrono::_V2::steady_clock::now();
    auto time = std::chrono::duration_cast< std::chrono::nanoseconds  , int64_t  , std::nano  > ((end-start)) . count();
    printf("Average execution time of MRC kernel: %f (us)\n",(time * 1e-3f / repeat));
    start = std::chrono::_V2::steady_clock::now();
    for (int i = 0; i <= repeat - 1; i += 1) {
      MRCGradient2(length,h_Y,h_X1,h_X2,h_O,m,h_dX1,h_dX2);
    }
    end = std::chrono::_V2::steady_clock::now();
    time = std::chrono::duration_cast< std::chrono::nanoseconds  , int64_t  , std::nano  > ((end-start)) . count();
    printf("Average execution time of MRC2 kernel: %f (us)\n",(time * 1e-3f / repeat));
  }
  reference(length,h_Y,h_X1,h_X2,h_O,m,r_dX1,r_dX2);
  bool ok = true;
  for (int i = 0; i <= length - 1; i += 1) {
    if ((std::fabs(h_dX1[i] - r_dX1[i])) > 1e-3 || (std::fabs(h_dX2[i] - r_dX2[i])) > 1e-3) {
      ok = false;
      break; 
    }
  }
  printf("%s\n",(ok?"PASS" : "FAIL"));
  free(h_X1);
  free(h_X2);
  free(h_O);
  free(h_Y);
  free(h_dX1);
  free(h_dX2);
  return 0;
}
