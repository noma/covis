// Copyright (c) 2015 Tobias Kramer <tobias.kramer@mytum.de>
//
// See accompanying file LICENSE and README for further information.

/* OpenCL kernel 
Compute gravitational dust trajectories orginating from triangular mesh cells

For the gravitational part we implement the algorithm published in

Conway, J. T. (2014). Analytical solution from vector potentials for the 
gravitational field of a general polyhedron. 
Celestial Mechanics and Dynamical Astronomy, 121(1), 17–38. 
http://doi.org/10.1007/s10569-014-9588-x
*/

#undef USING_DOUBLE_PRECISIONQ
#define USING_DOUBLE_PRECISIONQ

#ifdef USING_DOUBLE_PRECISIONQ
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#undef Real_t
#define Real_t double
#undef Real_t4
#define Real_t4 double4
#endif /* USING_DOUBLE_PRECISIONQ */

#define norm length

__kernel void integrate_eom( 
__global Real_t4 *pold, 
__global Real_t4 *vold, 
__global Real_t4 *pnew, 
__global Real_t4 *vnew, 
__global Real_t *nvIn,
__global Real_t *rijIn,
int numpoints, 
int numfaces, 
int numvertices,
Real_t dt,
Real_t omega,
Real_t gdens
)
{ 
/*
__global Real_t *phiOut, 
__global Real_t *gOut, 
__global Real_t *thetasum, 
*/    
   //for(int m=0;m<numpoints;m++)
   int m=get_global_id(0);

   {
      Real_t phi=0.0;
      Real_t thetasum=0.0;
      Real_t4 g=(Real_t4)(0.0,0.0,0.0,0.0);
      
      // Real_t4 Rm=(Real_t4)(RIn[3*m+0],RIn[3*m+1],RIn[3*m+2],0.0);

      Real_t4 Rm=pold[m];

      for(int i=0;i<numfaces;i++)
      {
         Real_t4 nv=(Real_t4)(nvIn[3*i+0],nvIn[3*i+1],nvIn[3*i+2],0.0);
         
         Real_t4  ri0=(Real_t4)(rijIn[(i*4)*3+0],rijIn[(i*4)*3+1],rijIn[(i*4)*3+2],0.0);
         Real_t4  rpi=nv*dot(nv,ri0)-cross(nv,cross(nv,Rm));
         
         Real_t4  r1=(Real_t4)(
                     rijIn[(i*4+0  )*3+0]-Rm.x,
                     rijIn[(i*4+0  )*3+1]-Rm.y,
                     rijIn[(i*4+0  )*3+2]-Rm.z,
                     0.0);
         Real_t4  r2=(Real_t4)(
                     rijIn[(i*4+1  )*3+0]-Rm.x,
                     rijIn[(i*4+1  )*3+1]-Rm.y,
                     rijIn[(i*4+1  )*3+2]-Rm.z,
                     0.0);
         Real_t4  r3=(Real_t4)(
                     rijIn[(i*4+2  )*3+0]-Rm.x,
                     rijIn[(i*4+2  )*3+1]-Rm.y,
                     rijIn[(i*4+2  )*3+2]-Rm.z,
                     0.0);
         Real_t nr1,nr2,nr3;
         nr1=norm(r1);
         nr2=norm(r2);
         nr3=norm(r3);
         // compute solid angle to determine if position is inside the comet or outside
         thetasum+=2.0*atan2(
                    dot(r1,cross(r2,r3)),
                    nr1*nr2*nr3
                    +dot(r1,r2)*nr3
                    +dot(r1,r3)*nr2
                    +dot(r2,r3)*nr1
                    );
         for(int j=0;j<numvertices;j++)
         {
            Real_t4 rij  =(Real_t4)(rijIn[(i*4+j  )*3+0],rijIn[(i*4+j  )*3+1],rijIn[(i*4+j  )*3+2],0.0); 
            Real_t4 rijp1=(Real_t4)(rijIn[(i*4+j+1)*3+0],rijIn[(i*4+j+1)*3+1],rijIn[(i*4+j+1)*3+2],0.0); 
            
            Real_t4 vsub_rijp1_rij=rijp1-rij;
            Real_t4 vsub_rij_rpi  =rij-rpi;
            Real_t4 vsub_rijp1_rpi=rijp1-rpi;
            Real_t4 vsub_Rm_rij   =Rm-rij;
            
            Real_t nrijp1rij = norm(vsub_rijp1_rij);
            Real_t theta = -sign(dot(nv,cross(vsub_rij_rpi,vsub_rijp1_rpi)));
            Real_t aux_norm=norm(vsub_rij_rpi)*norm(vsub_rijp1_rpi);
            
            if(aux_norm!=0.0)
            {
               Real_t arg=dot(vsub_rij_rpi,vsub_rijp1_rpi)/aux_norm;
               Real_t aux;
               
               if      (fabs(arg-1.0)<1.0e-12) aux=0.0;
               else if (fabs(arg+1.0)<1.0e-12) aux=3.1415926535897932385;
               else                     aux=acos(arg);
#ifdef PRINTF               
               if (isnan(aux)) 
                  printf("acos error\n");
#endif               
               theta*=aux;
            }
            
            Real_t Kij,a,b,c,dij,Iij;
            
            Kij=-fabs(dot(nv,vsub_Rm_rij))*theta;
            a = norm(vsub_Rm_rij)/nrijp1rij;
            b = dot(vsub_Rm_rij,vsub_rijp1_rij)/(nrijp1rij*nrijp1rij);
            c = dot(nv,vsub_Rm_rij)/nrijp1rij;
            dij = dot(cross(nv,vsub_Rm_rij),vsub_rijp1_rij)/nrijp1rij;
         
            if(fabs(dij)>1.0e-5)
            {
               Real_t epa2m2b=(1.0+a*a-2.0*b);
               Real_t a2mb2mc2=(a*a-b*b-c*c);
               
               Real_t sepa2m2b;
               Real_t sa2mb2mc2;
               
               if (epa2m2b<0.0) 
               { 
#ifdef PRINTF
                  printf("TROUBLE A\n");
#endif
                  sepa2m2b=1.0; 
               } 
               else 
                  sepa2m2b=sqrt(epa2m2b);
               if (a2mb2mc2<0.0) 
               {
#ifdef PRINTF
                  printf("TROUBLE B\n");
#endif
                  sa2mb2mc2=1.0; 
               } 
               else 
                  sa2mb2mc2=sqrt(a2mb2mc2);
               
               Iij=dij*((c*atan((b*c)/(a*sa2mb2mc2)))/sa2mb2mc2 + (c*atan(((1.0 - b)*c)/(sa2mb2mc2*sepa2m2b)))/sa2mb2mc2 + log((1.0 - b + sepa2m2b)/(a - b)));
            } 
            else
               Iij=0.0;  
            phi+=0.5*dot(nv,vsub_Rm_rij)*(Iij + Kij);
            g+=nv*(Iij+Kij);
         }
      }

      if (thetasum<0.1) // position outside the comet
      {
         g*=gdens;
         g.x+=(+2.0*omega*vold[m].y+pold[m].x*omega*omega);
         g.y+=(-2.0*omega*vold[m].x+pold[m].y*omega*omega);
         vnew[m]=vold[m]+g*dt;
         pnew[m]=pold[m]+vnew[m]+g*dt*dt*0.5;
      }
      else // we re-collided with the comet, do not update position, but mask vel.w as a hit (1.0)
      {
         vnew[m]=vold[m];
         vnew[m].w=1.0;
         pnew[m]=pold[m];
      }
   }  
}
