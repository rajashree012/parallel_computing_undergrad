#include<stdio.h>
#include<time.h>
#include<math.h>
#define X 0
#define Y 1
#define max 1000
main()
{
int n=0,i,j,xdst,ydst,k;
double time=0,dist,timstp=0.05,dc;
double cpu_time;
double G;
int pos[max][2],vel[max][2],force[max][2],force_x[max][max],force_y[max][max],m[max];
clock_t start,end;
G=6.673*pow(10,-11);
while(n<1000)
{

for(i=0;i<n;i++)
{
m[i]=rand()%1000;
	for(j=0;j<2;j++)
	{
		pos[i][j]=rand()%1000;
		vel[i][j]=rand()%1000;
		force[i][j]=0;
	}
}
start=clock();
while(time<=2000)
{
for(j=0;j<n;j++)
 {
	while(k>j)
	 {
		xdst=pos[j][X]-pos[k][X];
		ydst=pos[j][Y]-pos[k][Y];
		dist=sqrt(xdst*xdst+ydst*ydst);
		dc=dist*dist*dist;
		force_x[j][k]=G*m[j]*m[k]*xdst/(dc);
		force_y[j][k]=G*m[j]*m[k]*ydst/(dc);
		force[j][X]+=force_x[j][k];
		force[j][Y]+=force_y[j][k];
		force[k][X]-=force_x[j][k];
		force[k][Y]-=force_y[j][k];
		k++;

	}
}

for(j=0;j<n;j++)
{
pos[j][X]+=timstp*vel[j][X];
pos[j][Y]+=timstp*vel[j][Y];
vel[j][X]+=(timstp/m[j])*force[j][X];
vel[j][Y]+=(timstp/m[j])*force[j][Y];
}
time=time+timstp;
}
end=clock();
cpu_time=((double)(end-start))/CLOCKS_PER_SEC;
printf("for %d particles  %f",n,cpu_time);
n++;
time=0;
}
}
