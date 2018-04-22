#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "vector.h"

class MATRIX
{

	VECTOR m[3];

public:

	void transpose();
	
	      VECTOR &operator[](const int i)       { return m[i]; }
	const VECTOR &operator[](const int i) const { return m[i]; }


	MATRIX operator * (const MATRIX &b) 
        {
			MATRIX mr;    		           
               for (int i=0; i<3; i++){
                   for (int j=0; j<3; j++)
                   {
     	                mr[i][j] = m[i][0] * b[0][j]
                                + m[i][1] * b[1][j]
                                + m[i][2] * b[2][j];
                   }
               }
               return mr;
        }

	MATRIX(	const double a11, const double a12, const double a13,
		const double a21, const double a22, const double a23,
		const double a31, const double a32, const double a33)
	{
		m[0][0]=a11; m[0][1]=a12; m[0][2]=a13;
		m[1][0]=a21; m[1][1]=a22; m[1][2]=a23;
		m[2][0]=a31; m[2][1]=a32; m[2][2]=a33;
	}

		VECTOR vr;
        VECTOR operator * (const VECTOR &v) 
        {
           
           vr[0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0];
           vr[1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1];
           vr[2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2];
           return vr;
        };

	 MATRIX() {}
	~MATRIX() {}
};

static MATRIX t;
   
inline MATRIX translate(double x, double y, double z)
{

   t[0][0] = 1;   t[0][1] = 0;   t[0][2] = 0;   t[0][3] = 0;
   t[1][0] = 0;   t[1][1] = 1;   t[1][2] = 0;   t[1][3] = 0;
   t[2][0] = 0;   t[2][1] = 0;   t[2][2] = 1;   t[2][3] = 0;
   t[3][0] = -x;  t[3][1] = -y;  t[3][2] = -z;  t[3][3] = 1;
   return t;
}

static MATRIX r;

inline MATRIX transpose(const MATRIX &b) 
{

	r[0][0] = b[0][0];
	r[0][1] = b[1][0];
	r[0][2] = b[2][0];

	r[1][0] = b[0][1];
	r[1][1] = b[1][1];
	r[1][2] = b[2][1];

	r[2][0] = b[0][2];
	r[2][1] = b[1][2];
	r[2][2] = b[2][2];
	return r;
}

MATRIX rotX(const double theta)
{
	const double c = cos(theta);
	const double s = sin(theta);
	return MATRIX(	 1, 0, 0,
			 0, c, s,
			 0,-s, c);
}

MATRIX rotY(const double theta)
{
	const double c = cos(theta);
	const double s = sin(theta);
	return MATRIX(	 c, 0,-s,
			 0, 1, 0,
			 s, 0, c);
}

MATRIX rotZ(const double theta)
{
	const double c = cos(theta);
	const double s = sin(theta);
	return MATRIX(	 c, s, 0,
			-s, c, 0,
			 0, 0, 1);
}

#endif
