/********************************************************************************/

//
// DexAnalogMixin.h 
// Routines to process data from the Dex (Grip) apparatus.
//

#pragma once

#include "..\Useful\VectorsMixin.h"

#define N_FORCE_TRANSDUCERS		2
#define DEFAULT_COP_THRESHOLD	0.25

#define LEFT_ATI	0
#define RIGHT_ATI	1
#define LEFT_ATI_ROTATION	22.5
#define RIGHT_ATI_ROTATION	22.5

class DexAnalogMixin : public VectorsMixin {


public:

	// Constructor
	DexAnalogMixin( void );

	// Saves force values between calls, so that recursive filtering 
	// can be applied.
	double		filteredLoad;
	double		filteredGrip;

	// Structures needed to use the ATI Force/Torque transducer library.
	Quaternion			ftAlignmentQuaternion[N_FORCE_TRANSDUCERS];

	// Defines the rotation of each ATI sensor around the local Z axis.
	double	ATIRotationAngle[N_FORCE_TRANSDUCERS];

	// Methods to deal with the manipulandum force/torque measurements.	
	
	double ComputeCoP( Vector3 &cop, Vector3 &force, Vector3 &torque, double threshold = DEFAULT_COP_THRESHOLD );
	double ComputeGripForce( Vector3 &force1, Vector3 &force2 );	
	double ComputeLoadForce( Vector3 &load, Vector3 &force1, Vector3 &force2 );
	double ComputePlanarLoadForce( Vector3 &load, Vector3 &force1, Vector3 &force2 );

	double FilteredLoad( float new_load, float filter_constant );
	double FilteredGrip( float new_grip, float filter_constant );

};
