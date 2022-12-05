/*

	COORDSTR.CPP

	Coordinates with strategies

*/
	#include "3dc.h"
	#include "inline.h"

	#include "coordstr.hpp"

#if 0
	#include "davemcro.h"
	#include "scrobj.hpp"
#endif

	
		#define UseLocalAssert Yes
		#include "ourasert.h"
	
#if 0 	
 	static CoordinateWithStrategy* CoordinateWithStrategy::pCWS_FirstActive = NULL;
#endif

	#define INT_SECONDS_FOR_STANDARD_HOMING		(0.25)
	#define FIXP_SECONDS_FOR_STANDARD_HOMING	(ONE_FIXED * INT_SECONDS_FOR_STANDARD_HOMING)

	#define DAVEMCRO_NONZERO_AND_OPPOSITESIGN(Int1, Int2)	\
	(													\
		(												\
			((Int1) > 0)								\
			&&											\
			((Int2) < 0)								\
		)												\
		||												\
		(												\
			((Int1) < 0)								\
			&&											\
			((Int2) > 0)								\
		)												\
	)													

	

CoordinateWithStrategy::CoordinateWithStrategy
(
	int Int_InitialCoord,
	OurBool fActive
) : Daemon
	(
		fActive
	)
{
	Int_CurrentCoord_Val = Int_InitialCoord;
}


CoordinateWithStrategy::~CoordinateWithStrategy
(
)
{
}


int CoordinateWithStrategy::GetCoord_Int(void)
{
	return(Int_CurrentCoord_Val);
}

void CoordinateWithStrategy::SetCoord_Int(int Int_NewCoord)
{
	Int_CurrentCoord_Val = Int_NewCoord;
}


// Coordinates with velocity ///////////////////////////////////////////////
CoordinateWithVelocity :: CoordinateWithVelocity
(
	int Int_InitialCoord,
	int FixP_Velocity,
	OurBool fActive
) :	CoordinateWithStrategy
	(
		Int_InitialCoord,
		fActive
	)
{
	FixP_Velocity_Val	= FixP_Velocity;
	FixP_Position_Val	= OUR_INT_TO_FIXED( Int_CurrentCoord_Val );
}

CoordinateWithVelocity :: ~CoordinateWithVelocity()
{
	// empty
}

void CoordinateWithVelocity :: SetCoord_Int
(
	int Int_NewCoord
)
{
	Int_CurrentCoord_Val	= Int_NewCoord;
	FixP_Position_Val		= OUR_INT_TO_FIXED( Int_CurrentCoord_Val );
}

void CoordinateWithVelocity :: SetCoord_FixP
(
	int FixP_NewCoord
)
{
	Int_CurrentCoord_Val	= OUR_FIXED_TO_INT( FixP_NewCoord );
	FixP_Position_Val		= FixP_NewCoord;
}

void CoordinateWithVelocity :: SetVelocity_FixP(int FixP_NewVelocity)
{
	FixP_Velocity_Val = FixP_NewVelocity;
}


void CoordinateWithVelocity :: ApplyVelocity
(
	int FixP_Time
)
{
	FixP_Position_Val += MUL_FIXED(FixP_Time, FixP_Velocity_Val);

	Int_CurrentCoord_Val = OUR_FIXED_TO_INT( FixP_Position_Val );
}

int CoordinateWithVelocity :: GetCoord_Int_RoundedUp(void)
{

	int ReturnVal = (FixP_Position_Val >> 16);

    if ( (FixP_Position_Val&0xffff) )
    {
    	ReturnVal++;
	}

	#if 0
	textprint
	(
		"CoordinateWithVelocity :: GetCoord_Int_RoundedUp()\n"
		"FixP_Position_Val = %i\n"
		"ReturnVal = %i\n",
		FixP_Position_Val,
		ReturnVal
	);
	#endif

	return ReturnVal;
}

// Coordinates with velocity: pulsing /////////////////////////////////////////////
PulsingCoordinate :: PulsingCoordinate
(
	int Int_InitialCoord,
	int Int_SecondCoord,
	int FixP_Velocity,
	OurBool fActive
) :	CoordinateWithVelocity
	(
		Int_InitialCoord,
		FixP_Velocity,
		fActive
	)	
{
	Int_Target0_Val = Int_InitialCoord;
	FixP_Target0_Val = OUR_INT_TO_FIXED( Int_Target0_Val );

	Int_Target1_Val	= Int_SecondCoord;
	FixP_Target1_Val = OUR_INT_TO_FIXED( Int_Target1_Val );

}

PulsingCoordinate :: ~PulsingCoordinate()
{
	// empty
}

// Pulsing coordinates: acyclic ///////////////////////////////////////////////////
AcyclicPulsingCoordinate :: AcyclicPulsingCoordinate
(
	int Int_InitialCoord,
	int Int_SecondCoord,
	int FixP_Velocity,
	OurBool fActive
) : PulsingCoordinate
	(
		Int_InitialCoord,
		Int_SecondCoord,
		FixP_Velocity,
		fActive
	)
{
	// empty
}

AcyclicPulsingCoordinate :: ~AcyclicPulsingCoordinate()
{
	// empty
}


ACTIVITY_RETURN_TYPE AcyclicPulsingCoordinate :: Activity(ACTIVITY_INPUT)
{
	#if 0
	textprint("AcyclicPulsingCoordinate :: Activity(%i)\n", FixP_Time);
	#endif

	int Int_CurrentCoord_Old = Int_CurrentCoord_Val;

	ApplyVelocity( FixP_Time );

	int FixP_Displacement = (FixP_Target1_Val - FixP_Position_Val);

	#if 0
	textprint
	(
		"FixP_Velocity_Val = %i FixP_Displacement = %i\n",
		FixP_Velocity_Val,
		FixP_Displacement
	);
	#endif

	if 
	(
		DAVEMCRO_NONZERO_AND_OPPOSITESIGN
		(
			FixP_Velocity_Val,
			FixP_Displacement
		)
	)
	{
		{
			SetCoord_FixP
			(
				FixP_Target0_Val
				+
				FixP_Displacement
			);			
		}
	}

	ACTIVITY_RVAL_BOOL(Int_CurrentCoord_Old != Int_CurrentCoord_Val)
}

// Pulsing coordinates: acyclic ///////////////////////////////////////////////////
CyclicPulsingCoordinate :: CyclicPulsingCoordinate
(
	int Int_InitialCoord,
	int Int_SecondCoord,
	int FixP_Velocity,
	OurBool fActive
) : PulsingCoordinate
	(
		Int_InitialCoord,
		Int_SecondCoord,
		FixP_Velocity,
		fActive
	)
{
	fGoingForSecondCoord = Yes;
}

CyclicPulsingCoordinate :: ~CyclicPulsingCoordinate()
{
	// empty
}

ACTIVITY_RETURN_TYPE CyclicPulsingCoordinate :: Activity(ACTIVITY_INPUT)
{
	#if 0
	textprint("CyclicPulsingCoordinate :: Activity(%i)\n", FixP_Time);
	#endif

	int Int_CurrentCoord_Old = Int_CurrentCoord_Val;
	
	ApplyVelocity( FixP_Time );
	
	int FixP_Displacement =
	(
		( fGoingForSecondCoord ? FixP_Target1_Val : FixP_Target0_Val ) - FixP_Position_Val
	);

	#if 0
	textprint
	(
		"FixP_Velocity_Val = %i FixP_Displacement = %i\n",
		FixP_Velocity_Val,
		FixP_Displacement
	);
	#endif

	if 
	(
		DAVEMCRO_NONZERO_AND_OPPOSITESIGN
		(
			FixP_Velocity_Val,
			FixP_Displacement
		)
	)
	{
		{
			SetCoord_FixP
			(
				( fGoingForSecondCoord ? FixP_Target1_Val : FixP_Target0_Val )
				+
				FixP_Displacement
			);

			fGoingForSecondCoord = !fGoingForSecondCoord;

			SetVelocity_FixP
			(
				-FixP_Velocity_Val
			);
		}
	}

	ACTIVITY_RVAL_BOOL(Int_CurrentCoord_Old != Int_CurrentCoord_Val)
}



#if 1
HomingCoordinate::HomingCoordinate
(
	int Int_InitialCoord,
	int Int_TargetCoord
) : CoordinateWithVelocity
	(
		Int_InitialCoord,
		0, //FixP_Velocity,
		(Int_InitialCoord != Int_TargetCoord)
	)
{
	Int_TargetCoord_Val = Int_TargetCoord;
	FixP_TargetCoord_Val = OUR_INT_TO_FIXED( Int_TargetCoord );
	
	FixP_IdealVelocity_Val = DIV_FIXED
	(
		(FixP_TargetCoord_Val - FixP_Position_Val),
		FIXP_SECONDS_FOR_STANDARD_HOMING
	);
}

void HomingCoordinate :: SetCoord_Int(int Int_NewCoord)
{
	CoordinateWithVelocity :: SetCoord_Int
	(
		Int_NewCoord
	);

	SetActive( Int_TargetCoord_Val != GetCoord_Int() );	
}

void HomingCoordinate :: SetCoord_FixP(int FixP_NewCoord)
{
	CoordinateWithVelocity :: SetCoord_FixP
	(
		FixP_NewCoord
	);

	SetActive( Int_TargetCoord_Val != GetCoord_Int() );	
}


AcyclicHomingCoordinate :: AcyclicHomingCoordinate
(
	int Int_InitialCoord,
	int Int_TargetCoord
) : HomingCoordinate
	(
		Int_InitialCoord,
		Int_TargetCoord
	)
{
	// empty
}

ACTIVITY_RETURN_TYPE AcyclicHomingCoordinate :: Activity(ACTIVITY_INPUT)
{
	int Int_CurrentCoord_Old = Int_CurrentCoord_Val;

	#if 0
	textprint("acyclic homing coord for time %i\n",FixP_Time);
	#endif
	
	int FixP_Delta_Homing = (FixP_TargetCoord_Val - FixP_Position_Val);

	#if 0
	textprint
	(
		"current = %6i fixp = %6i\n",
		(int)Int_CurrentCoord_Val,
		(int)FixP_Position_Val
	);

	textprint
	(
		"target = %6i fixp = %6i\n",
		(int)Int_TargetCoord_Val,
		(int)FixP_TargetCoord_Val
	);
	#endif

	SetVelocity_FixP( FixP_IdealVelocity_Val );
	int FixP_Delta_Position = MUL_FIXED(FixP_Time, FixP_Velocity_Val);
	
	#if 0
	textprint
	(
		"FixP delta Homing= %6i PosN=%6i\n",
		(int)FixP_Delta_Homing,
		(int)FixP_Delta_Position		
	);
	#endif

		
	//if target is within this frame's velocity...
	if
	(
		abs(FixP_Delta_Homing)
		<=
		abs(FixP_Delta_Position)
	)
	{
		#if 0
		textprint("within range\n");
		#endif
		
#if 1
		SetCoord_Int
		(
			Int_TargetCoord_Val
		);
		SetVelocity_FixP( 0 );
		Stop();
#else
		ApplyVelocity(FixP_Time);
#endif
	}
	else
	{
		#if 0
		textprint("not within range\n");
		#endif
		
		ApplyVelocity(FixP_Time);
		
		
	}

	ACTIVITY_RVAL_BOOL(Int_CurrentCoord_Old != Int_CurrentCoord_Val)

}




#if 0
OurBool AcyclicHomingCoordinate::Activity(int FixP_Time)
{
	#if 1
	textprint("acyclic homing coord for time %i\n",FixP_Time);
	#endif
}
#endif


void AcyclicHomingCoordinate :: SetTarget_Int
(
	int Int_TargetCoord
)
{
	#if 0
	{
		char temp[100];
		sprintf
		(
			temp,
			"SetTarget(Int_=%i)",
			Int_TargetCoord			
		);		
		DAVELOG(temp);
	}
	#endif
	#if 0
	{
		textprint
		(
			"SetTarget(Int_=%i)\n",
			Int_TargetCoord			
		);		
	}
	#endif

	Int_TargetCoord_Val = Int_TargetCoord;
	FixP_TargetCoord_Val = OUR_INT_TO_FIXED( Int_TargetCoord );
	
	FixP_IdealVelocity_Val = DIV_FIXED(
		(FixP_TargetCoord_Val - FixP_Position_Val),
		FIXP_SECONDS_FOR_STANDARD_HOMING
	);

	
	SetActive( Int_TargetCoord != GetCoord_Int() );	
}

#if 0
OurBool AcyclicHomingCoordinate :: fTargetWithinThisFramesRange(void)
{
	
}

void AcyclicHomingCoordinate :: ChangeVelocityBasedOnHoming(int FixP_Time)
{
}
#endif

// class AcyclicFixedSpeedHoming : public CoordinateWithVelocity
// public:
AcyclicFixedSpeedHoming :: AcyclicFixedSpeedHoming
(
	int Int_InitialCoord,
	int Int_TargetCoord,
	int FixP_Speed_New
		// must be >= zero
) : CoordinateWithVelocity
	(
		Int_InitialCoord, // int Int_InitialCoord,
		(
			(Int_TargetCoord > Int_InitialCoord )
			?
			( FixP_Speed_New )
			:
			( -FixP_Speed_New )
		), // int FixP_Velocity,
		( Int_InitialCoord != Int_TargetCoord ) // OurBool fActive
	)
{
	GLOBALASSERT( FixP_Speed_New >= 0);

	FixP_Speed_Val = FixP_Speed_New;
}

AcyclicFixedSpeedHoming :: ~AcyclicFixedSpeedHoming()
{
}

ACTIVITY_RETURN_TYPE AcyclicFixedSpeedHoming :: Activity(ACTIVITY_INPUT)
{
	int Int_CurrentCoord_Old = Int_CurrentCoord_Val;

	#if 0
	textprint("acyclic homing coord for time %i\n",FixP_Time);
	#endif
	
	int FixP_Delta_Homing = (FixP_TargetCoord_Val - FixP_Position_Val);

	#if 0
	textprint
	(
		"current = %6i fixp = %6i\n",
		(int)Int_CurrentCoord_Val,
		(int)FixP_Position_Val
	);

	textprint
	(
		"target = %6i fixp = %6i\n",
		(int)Int_TargetCoord_Val,
		(int)FixP_TargetCoord_Val
	);
	#endif

	#if 0
	SetVelocity_FixP( FixP_IdealVelocity_Val );
	#endif

	int FixP_Delta_Position = MUL_FIXED(FixP_Time, FixP_Velocity_Val);
	
	#if 0
	textprint
	(
		"FixP delta Homing= %6i PosN=%6i\n",
		(int)FixP_Delta_Homing,
		(int)FixP_Delta_Position		
	);
	#endif

		
	//if target is within this frame's velocity...
	if
	(
		abs(FixP_Delta_Homing)
		<=
		abs(FixP_Delta_Position)
	)
	{
		#if 0
		textprint("within range\n");
		#endif
		
		SetCoord_FixP
		(
			FixP_TargetCoord_Val
		);
		SetVelocity_FixP( 0 );
		Stop();
	}
	else
	{
		#if 0
		textprint("not within range\n");
		#endif
		
		ApplyVelocity(FixP_Time);
		
		
	}

	ACTIVITY_RVAL_BOOL(Int_CurrentCoord_Old != Int_CurrentCoord_Val)
}

void AcyclicFixedSpeedHoming :: SetTarget_Int
(
	int Int_TargetCoord
)
{
	Int_TargetCoord_Val = Int_TargetCoord;
	FixP_TargetCoord_Val = OUR_INT_TO_FIXED( Int_TargetCoord );
	
	#if 1
	SetVelocity_FixP
	(
		(Int_TargetCoord_Val > GetCoord_Int() )
		?
		( FixP_Speed_Val )
		:
		( -FixP_Speed_Val )
	);
	#else
	if (FixP_TargetCoord_Val > FixP_Position_Val)
	{
		FixP_IdealVelocity_Val = FixP_IdealSpeed;
	}
	else
	{
		FixP_IdealVelocity_Val = -FixP_IdealSpeed;
	}
	#endif
	
	SetActive( Int_TargetCoord != GetCoord_Int() );	
}

void AcyclicFixedSpeedHoming :: SetTarget_FixP
(
	int FixP_TargetCoord
)
{
	Int_TargetCoord_Val = OUR_FIXED_TO_INT( FixP_TargetCoord );
	FixP_TargetCoord_Val = FixP_TargetCoord;
	
	#if 1
	SetVelocity_FixP
	(
		( FixP_TargetCoord_Val > FixP_Position_Val )
		?
		( FixP_Speed_Val )
		:
		( -FixP_Speed_Val )
	);
	#else
	if (FixP_TargetCoord_Val > FixP_Position_Val)
	{
		FixP_IdealVelocity_Val = FixP_IdealSpeed;
	}
	else
	{
		FixP_IdealVelocity_Val = -FixP_IdealSpeed;
	}
	#endif
	
	SetActive( FixP_TargetCoord != GetCoord_FixP() );
}


void AcyclicFixedSpeedHoming :: SetSpeed_FixP
(
	int FixP_Speed_New
		// must be >= zero
)
{
	GLOBALASSERT( FixP_Speed_New >= 0);

	FixP_Speed_Val = FixP_Speed_New;

	SetVelocity_FixP
	(
		( FixP_TargetCoord_Val > FixP_Position_Val )
		?
		( FixP_Speed_Val )
		:
		( -FixP_Speed_Val )
	);

}

#endif
