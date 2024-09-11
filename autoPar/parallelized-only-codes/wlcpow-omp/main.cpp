#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <chrono>
#include <omp.h>
#include "utils.h"
#define n_type 32
#include <omp.h> 

void bond_wlcpowallvisc(r64 *force_x,r64 *force_y,r64 *force_z,const float4 *coord_merged,const float4 *veloc,const int *nbond,const int2 *bonds,const r64 *bond_r0,const r32 *temp_global,const r32 *r0_global,const r32 *mu_targ_global,const r32 *qp_global,const r32 *gamc_global,const r32 *gamt_global,const r32 *sigc_global,const r32 *sigt_global,const float3 period,const int padding,const int n_local,const int teams,const int blocks)
{
{
    r32 shared_data[264];
// static size
{
      int threadIdx_x = omp_get_thread_num();
      int blockIdx_x = omp_get_team_num();
      r32 *temp = &shared_data[0];
      r32 *r0 = &shared_data[1 * (32 + 1)];
      r32 *mu_targ = &shared_data[2 * (32 + 1)];
      r32 *qp = &shared_data[3 * (32 + 1)];
      r32 *gamc = &shared_data[4 * (32 + 1)];
      r32 *gamt = &shared_data[5 * (32 + 1)];
      r32 *sigc = &shared_data[6 * (32 + 1)];
      r32 *sigt = &shared_data[7 * (32 + 1)];
      
#pragma omp parallel for
      for (int i = threadIdx_x; i <= 32; i += blocks) {
        temp[i] = temp_global[i];
        r0[i] = r0_global[i];
        mu_targ[i] = mu_targ_global[i];
        qp[i] = qp_global[i];
        gamc[i] = gamc_global[i];
        gamt[i] = gamt_global[i];
        sigc[i] = sigc_global[i];
        sigt[i] = sigt_global[i];
      }
      for (int i = blockIdx_x * blocks + threadIdx_x; i <= n_local - 1; i += teams * blocks) {
        int n = nbond[i];
        float4 coord1 = coord_merged[i];
        float4 veloc1 = veloc[i];
        r32 fxi = 0.f;
        r32 fyi = 0.f;
        r32 fzi = 0.f;
        for (int p = 0; p <= n - 1; p += 1) {
          int j = bonds[i + p * padding] . x;
          int type = bonds[i + p * padding] . y;
          float4 coord2 = coord_merged[j];
          r32 delx = minimum_image(coord1 . x - coord2 . x,period . x);
          r32 dely = minimum_image(coord1 . y - coord2 . y,period . y);
          r32 delz = minimum_image(coord1 . z - coord2 . z,period . z);
          float4 veloc2 = veloc[j];
          r32 dvx = veloc1 . x - veloc2 . x;
          r32 dvy = veloc1 . y - veloc2 . y;
          r32 dvz = veloc1 . z - veloc2 . z;
          r32 l0 = bond_r0[i + p * padding];
          r32 ra = sqrtf(delx * delx + dely * dely + delz * delz);
          r32 lmax = l0 * r0[type];
          r32 rr = 1.0f / r0[type];
          r32 sr = (1.0f - rr) * (1.0f - rr);
          r32 kph = powf(l0,qp[type]) * temp[type] * (0.25f / sr - 0.25f + rr);
// mu is described in the papers
          r32 mu = 0.433f * (temp[type] * (- 0.25f / sr + 0.25f + 0.5f * rr / (sr * (1.0f - rr))) / (lmax * rr) + kph * (qp[type] + 1.0f) / powf(l0,qp[type] + 1.0f));
// 0.25 * sqrt(3)
          r32 lambda = mu / mu_targ[type];
          kph = kph / lambda;
          rr = ra / lmax;
          r32 rlogarg = powf(ra,qp[type] + 1.0f);
          r32 vv = (delx * dvx + dely * dvy + delz * dvz) / ra;
          if (rr >= 0.99) 
            rr = 0.99f;
          if (rlogarg < 0.01) 
            rlogarg = 0.01f;
          float4 wrr;
          r32 ww[3][3];
          for (int tes = 0; tes <= 2; tes += 1) {
            for (int see = 0; see <= 2; see += 1) {
              int v1 =  *((int *)(&veloc1 . w));
              int v2 =  *((int *)(&veloc2 . w));
              ww[tes][see] = ::gaussian_TEA_fast< 4 > (v1 > v2,v1 + tes,v2 + see);
            }
          }
          wrr . w = (ww[0][0] + ww[1][1] + ww[2][2]) / 3.0f;
          wrr . x = (ww[0][0] - wrr . w) * delx + 0.5f * (ww[0][1] + ww[1][0]) * dely + 0.5f * (ww[0][2] + ww[2][0]) * delz;
          wrr . y = 0.5f * (ww[1][0] + ww[0][1]) * delx + (ww[1][1] - wrr . w) * dely + 0.5f * (ww[1][2] + ww[2][1]) * delz;
          wrr . z = 0.5f * (ww[2][0] + ww[0][2]) * delx + 0.5f * (ww[2][1] + ww[1][2]) * dely + (ww[2][2] - wrr . w) * delz;
          r32 fforce = -temp[type] * (0.25f / (1.0f - rr) / (1.0f - rr) - 0.25f + rr) / lambda / ra + kph / rlogarg + (sigc[type] * wrr . w - gamc[type] * vv) / ra;
          r32 fxij = delx * fforce - gamt[type] * dvx + sigt[type] * wrr . x / ra;
          r32 fyij = dely * fforce - gamt[type] * dvy + sigt[type] * wrr . y / ra;
          r32 fzij = delz * fforce - gamt[type] * dvz + sigt[type] * wrr . z / ra;
          fxi += fxij;
          fyi += fyij;
          fzi += fzij;
        }
        force_x[i] += fxi;
        force_y[i] += fyi;
        force_z[i] += fzi;
      }
    }
  }
}
template < typename T >
T * resize ( int n ) {
  return ( T * ) malloc ( sizeof ( T ) * n );
}

int main(int argc,char *argv[])
{
  if (argc != 2) {
    printf("Usage: ./%s <repeat>\n",argv[0]);
    return 1;
  }
  const int repeat = atoi(argv[1]);
  int i;
// all the values are randomly initialized
  float3 period = {(0.5f), (0.5f), (0.5f)};
  int padding = 1;
  int n = 1e6;
// problem size
  float4 *coord_merged = ::resize< float4  > (n + 1);
  float4 *veloc = ::resize< float4  > (n + 1);
  int *nbond = ::resize< int  > (n + 1);
// set the sizes properly
  int2 *bonds = ::resize< int2  > (n + n + 1);
  r64 *bond_r0 = ::resize< r64  > (n + n + 1);
  r64 *force_x = ::resize< r64  > (n + 1);
  r64 *force_y = ::resize< r64  > (n + 1);
  r64 *force_z = ::resize< r64  > (n + 1);
  r32 *bond_l0 = ::resize< r32  > (n + 1);
  r32 *temp = ::resize< r32  > (n + 1);
  r32 *mu_targ = ::resize< r32  > (n + 1);
  r32 *qp = ::resize< r32  > (n + 1);
  r32 *gamc = ::resize< r32  > (n + 1);
  r32 *gamt = ::resize< r32  > (n + 1);
  r32 *sigc = ::resize< r32  > (n + 1);
  r32 *sigt = ::resize< r32  > (n + 1);
  std::mt19937 g(19937);
  class std::uniform_real_distribution< r64  > dist_r64(0.1,0.9);
  class std::uniform_real_distribution< r32  > dist_r32(0.1,0.9);
  class std::uniform_int_distribution< i32  > dist_i32(0,32);
  for (i = 0; i <= n + n + 1 - 1; i += 1) {
    bond_r0[i] = dist_r64(g) + 0.001;
// select two distinct atoms in the kernel to evaluate their forces
    bonds[i] = {(i + 1) % (n + 1), dist_i32(g)};
  }
  for (i = 0; i <= n + 1 - 1; i += 1) {
    force_x[i] = force_y[i] = force_z[i] = 0.f;
    nbond[i] = dist_i32(g);
    coord_merged[i] = {dist_r32(g), dist_r32(g), dist_r32(g), (0)};
    r32 vx = dist_r32(g);
    r32 vy = dist_r32(g);
    r32 vz = dist_r32(g);
    veloc[i] = {vx, vy, vz, sqrtf(vx * vx + vy * vy + vz * vz)};
    bond_l0[i] = dist_r32(g);
    gamt[i] = dist_r32(g);
    gamc[i] = (dist_i32(g) % 4 + 4) * gamt[i];
// gamt[i] <= 3.0*gamc[i]
    temp[i] = dist_r32(g);
    mu_targ[i] = dist_r32(g);
    qp[i] = dist_r32(g);
    sigc[i] = (sqrt(2.0 * temp[i] * (3.0 * gamc[i] - gamt[i])));
    sigt[i] = (2.0 * (std::sqrt(gamt[i] * temp[i])));
  }
{
    const int teams = (n + 127) / 128;
    const int blocks = 128;
    auto start = std::chrono::_V2::steady_clock::now();
// note the outputs are not reset for each run
    for (i = 0; i <= repeat - 1; i += 1) {
      bond_wlcpowallvisc(force_x,force_y,force_z,coord_merged,veloc,nbond,bonds,bond_r0,temp,bond_l0,mu_targ,qp,gamc,gamt,sigc,sigt,period,padding,n,teams,blocks);
    }
    auto end = std::chrono::_V2::steady_clock::now();
    auto time = std::chrono::duration_cast< std::chrono::nanoseconds  , int64_t  , std::nano  > ((end-start)) . count();
    printf("Average kernel execution time: %f (us)\n",(time * 1e-3f / repeat));
  }
// no NaN values in the outputs
  for (i = 0; i <= n + 1 - 1; i += 1) {
    bool r = std::isnan(force_x[i]) || std::isnan(force_y[i]) || std::isnan(force_z[i]);
    if (r) 
      printf("There are NaN numbers at index %d\n",i);
  }
  double force_x_sum = 0;
  double force_y_sum = 0;
  double force_z_sum = 0;
  
#pragma omp parallel for private (i) reduction (+:force_x_sum,force_y_sum,force_z_sum)
  for (i = 0; i <= n + 1 - 1; i += 1) {
    force_x_sum += force_x[i];
    force_y_sum += force_y[i];
    force_z_sum += force_z[i];
  }
// values are meaningless, but they should be consistent across devices
  printf("checksum: forceX=%lf forceY=%lf forceZ=%lf\n",force_x_sum / (n + 1),force_y_sum / (n + 1),force_z_sum / (n + 1));
#ifdef DEBUG
#endif
  free(coord_merged);
  free(veloc);
  free(force_x);
  free(force_y);
  free(force_z);
  free(nbond);
  free(bonds);
  free(bond_r0);
  free(bond_l0);
  free(temp);
  free(mu_targ);
  free(qp);
  free(gamc);
  free(gamt);
  free(sigc);
  free(sigt);
  return 0;
}
