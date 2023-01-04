#include "StdAfx.h"
#include ".\fftops.h"
#include <math.h>

#include <string>
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

BYTE CalcConfidenceFromFFT( double idx )
{
    if( idx < 0.00007 )
        return 100;
    if( idx < 0.00010 )
        return 99;
    if( idx < 0.00011 )
        return 98;
    if( idx < 0.00012 )
        return 97;
    if( idx < 0.00013 )
        return 96;
    if( idx < 0.00017 )
        return 95;
    if( idx < 0.00019 )
        return 94;
    if( idx < 0.00021 )
        return 93;
    if( idx < 0.00023 )
        return 92;
    if( idx < 0.00025 )
        return 91;
    if( idx < 0.00028 )
        return 90;
    if( idx < 0.00031 )
        return 89;
    if( idx < 0.00035 )
        return 88;
    if( idx < 0.00039 )
        return 87;
    if( idx < 0.00043 )
        return 86;
    if( idx < 0.00047 )
        return 85;
    if( idx < 0.00052 )
        return 84;
    if( idx < 0.00056 )
        return 83;
    if( idx < 0.00060 )
        return 82;
    if( idx < 0.00064 )
        return 81;
    if( idx < 0.00068 )
        return 80;
    if( idx < 0.00072 )
        return 79;
    if( idx < 0.00076 )
        return 78;
    if( idx < 0.00080 )
        return 77;
    if( idx < 0.00084 )
        return 76;
    if( idx < 0.00088 )
        return 75;
    if( idx < 0.00092 )
        return 74;
    if( idx < 0.00094 )
        return 73;
    if( idx < 0.00094 )
        return 72;
    if( idx < 0.001 )
        return 71;
    if( idx < 0.00106 )
        return 70;
    if( idx < 0.0011 )
        return 69;
    if( idx < 0.00115 )
        return 68;
    if( idx < 0.00120 )
        return 67;
    if( idx < 0.00125 )
        return 66;
    if( idx < 0.00130 )
        return 65;
    if( idx < 0.00135 )
        return 64;
    if( idx < 0.00140 )
        return 63;
    if( idx < 0.00145 )
        return 62;
    if( idx < 0.00150 )
        return 61;
    if( idx < 0.00155 )
        return 60;
    if( idx < 0.00160 )
        return 50;
    if( idx < 0.0017 )
        return 40;
    if( idx < 0.0018 )
        return 30;
    if( idx < 0.0019 )
        return 20;
    if( idx < 0.002 )
        return 10;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/*
    Code modified from author below:
    Function FFT2->FFT2Sigs was modified and FFT, DFT and IsPowerof2 are used as they are
    The FFT2 function was modified so as to generate the FFT values that we want,
    we discard the image contents afterward
    Removed memory allocation functions in favor of static arrays,
    Since we only use it to process monochrome 1bpp bitmaps, we only use two values 1 or 0, this 
    appears to make the operation somewhat faster
  --------------------------------------------------------------------------------

	  NOTICE, DISCLAIMER, and LICENSE:

	CxImage version 5.99c 17/Oct/2004

	CxImage :   (C) 2001 - 2004, Davide Pizzolato

	Original CImage and CImageIterator implementation are:
	  (C) 1995, Alejandro Aguilar Sierra (asierra(at)servidor(dot)unam(dot)mx)

	Covered code is provided under this license on an "as is" basis, without warranty
	of any kind, either expressed or implied, including, without limitation, warranties
	that the covered code is free of defects, merchantable, fit for a particular purpose
	or non-infringing. The entire risk as to the quality and performance of the covered
	code is with you. Should any covered code prove defective in any respect, you (not
	the initial developer or any other contributor) assume the cost of any necessary
	servicing, repair or correction. This disclaimer of warranty constitutes an essential
	part of this license. No use of any covered code is authorized hereunder except under
	this disclaimer.

	Permission is hereby granted to use, copy, modify, and distribute this
	source code, or portions hereof, for any purpose, including commercial applications,
	freely and without fee, subject to the following restrictions: 

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.

  --------------------------------------------------------------------------------

	Other information: about CxImage, and the latest version, can be found at the
	CxImage home page: http://www.xdp.it

  --------------------------------------------------------------------------------
*/

#ifndef PI
 #define PI 3.141592653589793f
#endif

////////////////////////////////////////////////////////////////////////////////
bool IsPowerof2(long x)
{
	long i=0;
	while ((1<<i)<x) i++;
	if (x==(1<<i)) return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of n=2^m points.
   o(n)=n*log2(n)
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
   Written by Paul Bourke, July 1998
   FFT algorithm by Cooley and Tukey, 1965 
*/
bool FFT(int dir,int m,double *x,double *y)
{
	long nn,i,i1,j,k,i2,l,l1,l2;
	double c1,c2,tx,ty,t1,t2,u1,u2,z;

	/* Calculate the number of points */
	nn = 1<<m;

	/* Do the bit reversal */
	i2 = nn >> 1;
	j = 0;
	for (i=0;i<nn-1;i++) {
		if (i < j) {
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0;
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0;
		u2 = 0.0;
		for (j=0;j<l1;j++) {
			for (i=j;i<nn;i+=l2) {
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1;
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1)
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

	/* Scaling for forward transform */
	if (dir == 1) {
		for (i=0;i<nn;i++) {
			x[i] /= (double)nn;
			y[i] /= (double)nn;
		}
	}

   return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
   Direct fourier transform o(n)=n^2
   Written by Paul Bourke, July 1998 
*/
bool DFT(int dir,long m,double *x1,double *y1,double *x2,double *y2)
{
   long i,k;
   double arg;
   double cosarg,sinarg;
   
   for (i=0;i<m;i++) {
      x2[i] = 0;
      y2[i] = 0;
      arg = - dir * 2.0 * PI * i / (double)m;
      for (k=0;k<m;k++) {
         cosarg = cos(k * arg);
         sinarg = sin(k * arg);
         x2[i] += (x1[k] * cosarg - y1[k] * sinarg);
         y2[i] += (x1[k] * sinarg + y1[k] * cosarg);
      }
   }
   
   /* Copy the data back */
   if (dir == 1) {
      for (i=0;i<m;i++) {
         x1[i] = x2[i] / m;
         y1[i] = y2[i] / m;
      }
   } else {
      for (i=0;i<m;i++) {
         x1[i] = x2[i];
         y1[i] = y2[i];
      }
   }
   
   return true;
}

    _complex grid[256][256];
    double real2[256];
    double imag2[256];
    double real[256];
    double imag[256];
    BYTE ffts[256*256*2];

bool FFT2Sigs(Magick::Image *image, double * pSignatures, int scale_factor, long direction, bool bForceFFT, bool bMagnitude )
{
	if (!image) return false;
    long w,h;
	w=image->columns();
	h=image->rows();

	int iPixelSize = 1; // 1=GrayScale, 3=24bpp, 4=32bpp
    int factor = scale_factor;
    int factor_minus = factor-1;
    int mid_factor = factor/2;
	
    int iBlockSize = 16;

    Magick::Geometry geometry( scale_factor, scale_factor );
    geometry.aspect(true);
	//resample for FFT, if necessary 
	//image->sample(geometry);
	image->scale(geometry); //simple algo
	w=image->columns();
	h=image->rows();
	h=scale_factor;
	w=scale_factor;

    memset((BYTE*)grid,0,256*256*sizeof(_complex));

  register const MagickLib::PixelPacket *p;
  for (int y=0; y < (long) h; y++)
  {
    p=image->getConstPixels(0,y,w,1);
    if (p == (const MagickLib::PixelPacket *) NULL)
      break;
    for (int x=0; x < (long) w; x++)
    {
      BYTE luma = (int)(MagickLib::ScaleQuantumToChar(p->red) * 0.3 + MagickLib::ScaleQuantumToChar(p->green) * 0.59 + MagickLib::ScaleQuantumToChar(p->blue) * 0.11);
	  grid[x][y].x = luma;
	  //grid[x][y].y = luma;
      p++;
    }
  }

	//ok, here we have a (w x h), grayscale image ready for a FFT

	long m;
	int j,k;

	m=0;
	while((1<<m)<w) m++;

	for (j=0;j<h;j++) {
		for (k=0;k<w;k++) {
			real[k] = grid[k][j].x;
			imag[k] = grid[k][j].x;
		}

		//if (bXpow2) 
            FFT(direction,m,real,imag);
		//else		
        //    DFT(direction,w,real,imag,real2,imag2);

		for (k=0;k<w;k++) {
			grid[k][j].x = real[k];
			grid[k][j].y = imag[k];
		}
	}

	m=0;
	while((1<<m)<h) m++;

	for (k=0;k<w;k++) {
		for (j=0;j<h;j++) {
			real[j] = grid[k][j].x;
			imag[j] = grid[k][j].y;
		}

		//if (bYpow2) 
            FFT(direction,m,real,imag);
		//else		
        //    DFT(direction,h,real,imag,real2,imag2);

		for (j=0;j<h;j++) {
			grid[k][j].x = real[j];
			grid[k][j].y = imag[j];
		}
	}

	// converting from double to byte, there is a HUGE loss in the dynamics
	//  "nn" tries to keep an acceptable SNR, but 8bit=48dB: don't ask more 
	double nn=pow((double)2,(double)log((double)max(w,h))/(double)log((double)2)-4);
	//reversed gain for reversed transform
	if (direction==-1) nn=1/nn;
	//bMagnitude : just to see it on the screen
	if (bMagnitude) nn*=4;

    UINT len = w * h;
    BYTE * rfft = (BYTE*)ffts;
    BYTE * ifft = (BYTE*)ffts+len;
    memset(ffts,0,len*2);

	for (j=0;j<h;j++) {
		for (k=0;k<w;k++) {
			if (bMagnitude){
                *rfft++ = (BYTE)max(0,min(factor_minus,(nn*(3+log(_cabs(grid[k][j]))))));
				if (grid[k][j].x==0){
                    *ifft++ = (BYTE)max(0,min(factor_minus,(mid_factor+(atan(grid[k][j].y/0.0000000001)*nn))));
				} else {
                    *ifft++ = (BYTE)max(0,min(factor_minus,(mid_factor+(atan(grid[k][j].y/grid[k][j].x)*nn))));
				}
			} else {
                *rfft++ = (BYTE)max(0,min(factor_minus,(mid_factor + grid[k][j].x*nn)));
                *ifft++ = (BYTE)max(0,min(factor_minus,(mid_factor + grid[k][j].y*nn)));
			}
		}
	}

	int iWidth = factor;
	int iHeight = factor;
	int iRow;
	int iCol;
	int iLoopRow;
	int iLoopCol;
	double dTemp;
	int iBlockNum = 0;
	long lVal1;
	BYTE *pImage = (BYTE*)ffts;

	for(iRow = 0; iRow < iHeight; iRow += iBlockSize)
	{		
		for(iCol = 0; iCol < iWidth; iCol += iBlockSize )
		{
			// Block Processing
			pSignatures[iBlockNum] = 0.0;
			dTemp = 0.0;
			for(iLoopRow = 0; iLoopRow < iBlockSize; iLoopRow++ )
			{
				for(iLoopCol = 0; iLoopCol < iBlockSize; iLoopCol++ )
				{
					//lVal1 = pImage[((iRow+iLoopRow)*iPixelSize*iWidth) + ((iCol+iLoopCol)*iPixelSize)];
					lVal1 = pImage[ ((iRow+iLoopRow)*iWidth) + (iCol+iLoopCol) ];
					dTemp += (double) lVal1;
				}
			}
			
			pSignatures[iBlockNum++] = sqrt(dTemp);
		}
	}
	
	//
	// Normalize
	//
	for(int i = 0; i < factor; i++ )
	{
		dTemp += pSignatures[i];
	}
	
	for(int i = 0; i < factor; i++ )
	{
		pSignatures[i] /= dTemp;
	}

	return true;
}
