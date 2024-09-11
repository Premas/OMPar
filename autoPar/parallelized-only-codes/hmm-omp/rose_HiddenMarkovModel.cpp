/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
#include <stdlib.h>
#include <stdio.h>
// forward declaractions
#include <omp.h> 
int initHMM(float *initProb,float *mtState,float *mtObs,const int &nState,const int &nEmit);
int ViterbiCPU(float &viterbiProb,int *viterbiPath,int *obs,const int &nObs,float *initProb,float *mtState,const int &nState,float *mtEmit);
int ViterbiGPU(float &viterbiProb,int * __restrict__ viterbiPath,int * __restrict__ obs,const int nObs,float * __restrict__ initProb,float * __restrict__ mtState,const int nState,const int nEmit,float * __restrict__ mtEmit);
// main function
//*****************************************************************************

int main(int argc,const char **argv)
{
  int nState = 4096;
// number of states, must be a multiple of 256
  int nEmit = 4096;
// number of possible observations
  int nDevice = 1;
  float *initProb = (float *)(malloc(sizeof(float ) * nState));
// initial probability
  float *mtState = (float *)(malloc(sizeof(float ) * nState * nState));
// state transition matrix
  float *mtEmit = (float *)(malloc(sizeof(float ) * nEmit * nState));
// emission matrix
  initHMM(initProb,mtState,mtEmit,nState,nEmit);
// define observational sequence
  int nObs = 500;
// size of observational sequence
  int **obs = (int **)(malloc(nDevice * sizeof(int *)));
  int **viterbiPathCPU = (int **)(malloc(nDevice * sizeof(int *)));
  int **viterbiPathGPU = (int **)(malloc(nDevice * sizeof(int *)));
  float *viterbiProbCPU = (float *)(malloc(nDevice * sizeof(float )));
  float *viterbiProbGPU = (float *)(malloc(nDevice * sizeof(float )));
  for (int iDevice = 0; iDevice <= nDevice - 1; iDevice += 1) {
    obs[iDevice] = ((int *)(malloc(sizeof(int ) * nObs)));
    
#pragma omp parallel for
    for (int i = 0; i <= nObs - 1; i += 1) {
      obs[iDevice][i] = i % 15;
    }
    viterbiPathCPU[iDevice] = ((int *)(malloc(sizeof(int ) * nObs)));
    viterbiPathGPU[iDevice] = ((int *)(malloc(sizeof(int ) * nObs)));
  }
  printf("# of states = %d\n# of possible observations = %d \nSize of observational sequence = %d\n\n",nState,nEmit,nObs);
  printf("\nCompute Viterbi path on GPU\n");
  for (int iDevice = 0; iDevice <= nDevice - 1; iDevice += 1) {
    ViterbiGPU(viterbiProbGPU[iDevice],viterbiPathGPU[iDevice],obs[iDevice],nObs,initProb,mtState,nState,nEmit,mtEmit);
  }
  printf("\nCompute Viterbi path on CPU\n");
  for (int iDevice = 0; iDevice <= nDevice - 1; iDevice += 1) {
    ViterbiCPU(viterbiProbCPU[iDevice],viterbiPathCPU[iDevice],obs[iDevice],nObs,initProb,mtState,nState,mtEmit);
  }
  bool pass = true;
  
#pragma omp parallel for firstprivate (nObs)
  for (int iDevice = 0; iDevice <= nDevice - 1; iDevice += 1) {
    for (int i = 0; i <= nObs - 1; i += 1) {
      if (viterbiPathCPU[iDevice][i] != viterbiPathGPU[iDevice][i]) {
        pass = false;
        break; 
      }
    }
  }
  if (pass) 
    printf("Success");
   else 
    printf("Fail");
  printf("\n");
  free(initProb);
  free(mtState);
  free(mtEmit);
  for (int iDevice = 0; iDevice <= nDevice - 1; iDevice += 1) {
    free(obs[iDevice]);
    free(viterbiPathCPU[iDevice]);
    free(viterbiPathGPU[iDevice]);
  }
  free(obs);
  free(viterbiPathCPU);
  free(viterbiPathGPU);
  free(viterbiProbCPU);
  free(viterbiProbGPU);
  return 0;
}
// initialize initial probability, state transition matrix and emission matrix with random 
// numbers. Note that this does not satisfy the normalization property of the state matrix. 
// However, since the algorithm does not use this property, for testing purpose this is fine.
//*****************************************************************************

int initHMM(float *initProb,float *mtState,float *mtObs,const int &nState,const int &nEmit)
{
  if (nState <= 0 || nEmit <= 0) 
    return 0;
// Initialize initial probability
  for (int i = 0; i <= nState - 1; i += 1) {
    initProb[i] = (rand());
  }
  float sum = 0.0;
  
#pragma omp parallel for reduction (+:sum)
  for (int i = 0; i <= nState - 1; i += 1) {
    sum += initProb[i];
  }
  
#pragma omp parallel for firstprivate (sum)
  for (int i = 0; i <= nState - 1; i += 1) {
    initProb[i] /= sum;
  }
// Initialize state transition matrix
  for (int i = 0; i <= nState - 1; i += 1) {
    for (int j = 0; j <= nState - 1; j += 1) {
      mtState[i * nState + j] = (rand());
      mtState[i * nState + j] /= 2147483647;
    }
  }
// init emission matrix
  for (int i = 0; i <= nEmit - 1; i += 1) {
    for (int j = 0; j <= nState - 1; j += 1) {
      mtObs[i * nState + j] = (rand());
    }
  }
// normalize the emission matrix
  
#pragma omp parallel for private (i_nom_12,i_nom_13)
  for (int j = 0; j <= nState - 1; j += 1) {
    float sum = 0.0;
    
#pragma omp parallel for reduction (+:sum)
    for (int i = 0; i <= nEmit - 1; i += 1) {
      sum += mtObs[i * nState + j];
    }
    
#pragma omp parallel for firstprivate (sum)
    for (int i = 0; i <= nEmit - 1; i += 1) {
      mtObs[i * nState + j] /= sum;
    }
  }
  return 1;
}
