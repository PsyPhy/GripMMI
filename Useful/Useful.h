#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

// Useful constants for accessing components of 
//  vectors and quaternions.
#define X	0
#define Y	1
#define Z	2
#define M	3

#define ROLL	0
#define PITCH	1
#define YAW		2

typedef double Vector3[3];
typedef float  Vector3f[3];
typedef double Quaternion[4];
typedef double Matrix3x3[3][3];

#define MISSING_DOUBLE	999999.999999
#define MISSING_FLOAT	999999.999999f
#define MISSING_CHAR	127

#define Pi	M_PI

#define UNDEFINED		-1