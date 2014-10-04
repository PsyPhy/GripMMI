/********************************************************************************/

//
// DexAnalogMixin.h 
// Routines to process data from the Dex (Grip) apparatus.
//

#pragma once

#include "..\Useful\VectorsMixin.h"

#define N_FORCE_TRANSDUCERS		2
#define DEFAULT_COP_THRESHOLD	0.25
#define DEFAULT_FILTER_CONSTANT	1.0

#define LEFT_ATI	0
#define RIGHT_ATI	1
#define LEFT_ATI_ROTATION	22.5
#define RIGHT_ATI_ROTATION	22.5

class DexAnalogMixin : public VectorsMixin {


public:

	// Constructor
	DexAnalogMixin( void );

	// Structures needed to use the ATI Force/Torque transducer library.
	Quaternion			ftAlignmentQuaternion[N_FORCE_TRANSDUCERS];

	// Defines the rotation of each ATI sensor around the local Z axis.
	double	ATIRotationAngle[N_FORCE_TRANSDUCERS];

	// Methods to deal with the manipulandum force/torque measurements.	
	
	double ComputeCoP( Vector3 &cop, Vector3 &force, Vector3 &torque, double threshold = DEFAULT_COP_THRESHOLD );
	double ComputeGripForce( Vector3 &force1, Vector3 &force2 );	
	double ComputeLoadForce( Vector3 &load, Vector3 &force1, Vector3 &force2 );
	double ComputePlanarLoadForce( Vector3 &load, Vector3 &force1, Vector3 &force2 );

	// Recursive filtering of certain vector and matrix quantities.

	// Saves force values between calls, so that recursive filtering 
	// can be applied.
	Vector3		filteredLoadForce;
	double		filteredGripForce;
	Vector3		filteredAcceleration;
	Vector3		filteredManipulandumPosition;
	Vector3		filteredCoP[N_FORCE_TRANSDUCERS];

	double		filterConstant;
	// The filter constant sets the amount of lowpass filtering.
	// Setting a high value filters a lot. Setting to 0 means no filtering.
	// Fractional values are permitted to achieve less filtering.
	// The effective cut-off frequency will depend on your samping rate.
	void SetFilterConstant( double constant = 0.0 ); // Default is no filtering.
	double GetFilterConstant( void );


	// Values that can be filtered. Note that vector values are filtered 'in place'
	//  meaning that the filtered value is returned in the vector that was used
	//  to input the new data sample. Vector routines, as a bonus, return the
	//  magnitude of the vector as a scalar return value.
	double FilterLoadForce( Vector3 load_force );
	double FilterCoP( int which_transducer, Vector3 center_of_pressure );
	double FilterManipulandumPosition( Vector3 position );

	double FilterGripForce( double grip_force );

};
