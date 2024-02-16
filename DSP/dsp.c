#include "dsp.h"



void dsp()
{
double a0[12] = {1.0037, 1.0036, 1.0071};
double a1[12] = {-2,-1.9999, -1.9996};
double a2[12] = {0.9963, 0.9964, 0.9929};
double b0[12] = {0.0037, 0.0036, 0.0071};
double b2[12] = {-0.0037, -0.0036, -0.0071};
double y[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
double y1[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
double y2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
double multipliers[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
double x0 = 0;
double x1 = 0;
double x2 = 0;
double outBuffer[100];

for(int i = 0; i < 100; i++)
{
    for(int z = 0; z < 12; z++)
    {
        y2[z] = y1[z];
        y1[z] = y[z];
        x2 = x1;
        x1 = x0;
        y[z] =( b0[z]*x0 + b2[z]*x2-(a1[z]*y1[z] + a2[z]*y2[z]))/a0[z];
        outBuffer[i] = outBuffer[i] - multipliers[z] * y[z];
    }


}

}