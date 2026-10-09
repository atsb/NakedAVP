/*
	
	coordstr.hpp

*/

#ifndef coordstr
#define coordstr 1

	#ifndef _daemon
	#include "daemon.h"
	#endif

/* Type definitions *****************************************************/

	class CoordinateWithStrategy : public Daemon
	{
		public:
			CoordinateWithStrategy
			(
				int Int_InitialCoord,
				OurBool fActive
			);

			virtual ~CoordinateWithStrategy();

			int GetCoord_Int(void);
			virtual void SetCoord_Int(int Int_NewCoord);

			virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT) = 0;
				// the strategy to run when active...

			#if 0
			virtual void UpdateScreenObject
			(
				ScreenObject* pScrObj
			) = 0;
			#endif

		protected:
			int Int_CurrentCoord_Val;
	};

#if 0
	class CoordinateWith2DStrategy : public Daemon
	{
		public
			CoordinateWith2DStrategy
	};    
#endif

	class CoordinateWithVelocity : public CoordinateWithStrategy
	{
		protected:
			void ApplyVelocity(int FixP_Time);
			int FixP_Velocity_Val;
			int FixP_Position_Val;
			
		public:
			CoordinateWithVelocity
			(
				int Int_InitialCoord,
				int FixP_Velocity,
				OurBool fActive
			);

			virtual ~CoordinateWithVelocity();

			virtual void SetCoord_Int(int Int_NewCoord);
			virtual void SetCoord_FixP(int FixP_NewCoord);

			void SetVelocity_FixP(int FixP_NewVelocity);

			virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT) = 0;

			int GetCoord_FixP(void) {return FixP_Position_Val;}
			int GetCoord_Int_RoundedUp(void);

	};

	class PulsingCoordinate : public CoordinateWithVelocity
	{
		public:
			PulsingCoordinate
			(
				int Int_InitialCoord,
				int Int_SecondCoord,
				int FixP_Velocity,
				OurBool fActive
			);

			virtual ~PulsingCoordinate();

			virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT) = 0;

		protected:
			int Int_Target0_Val;
			int FixP_Target0_Val;

			int Int_Target1_Val;
			int FixP_Target1_Val;
	};

	class AcyclicPulsingCoordinate : public PulsingCoordinate
	{
		public:
			AcyclicPulsingCoordinate
			(
				int Int_InitialCoord,
				int Int_SecondCoord,
				int FixP_Velocity,
				OurBool fActive
			);

			virtual ~AcyclicPulsingCoordinate();

			virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);
				// base class activity; derived classes should call this
	};

	class CyclicPulsingCoordinate : public PulsingCoordinate
	{
		public:
			CyclicPulsingCoordinate
			(
				int Int_InitialCoord,
				int Int_SecondCoord,
				int FixP_Velocity,
				OurBool fActive
			);

			virtual ~CyclicPulsingCoordinate();

			ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);
		private:
			OurBool fGoingForSecondCoord;

	};

#if 1
	class HomingCoordinate : public CoordinateWithVelocity
	{
		protected:
			int Int_TargetCoord_Val;
			int FixP_TargetCoord_Val;

			int FixP_IdealVelocity_Val;

		public:
			HomingCoordinate
			(
				int Int_InitialCoord,
				int Int_TargetCoord
			);

			int GetTarget_Int(void) const;

			void SetCoord_Int(int Int_NewCoord);
			void SetCoord_FixP(int FixP_NewCoord);

			#if 0
			virtual OurBool fTargetWithinThisFramesRange(void) = 0;
			virtual void ChangeVelocityBasedOnHoming(int FixP_Time) = 0;
			#endif
	};

		// Inline fns:
		inline int HomingCoordinate::GetTarget_Int(void) const {return Int_TargetCoord_Val;}



	class AcyclicHomingCoordinate : public HomingCoordinate
	{
		public:
			AcyclicHomingCoordinate
			(
				int Int_InitialCoord,
				int Int_TargetCoord
			);

			virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);

			void SetTarget_Int
			(
				int Int_TargetCoord
			);

			#if 0
			OurBool fTargetWithinThisFramesRange(void);
			void ChangeVelocityBasedOnHoming(int FixP_Time);
			#endif
	
	};

	class CyclicHomingCoordinate : public CoordinateWithStrategy
	{
		public:
			CyclicHomingCoordinate
			(
				int Int_InitialCoord,
				int Int_TargetCoord,
				int Int_MinVal,
				int Int_MaxVal
			);

			void SetTarget_Int
			(
				int Int_TargetCoord
			);
			
		private:
			int Int_TargetCoord_Val;
	};

	class AcyclicFixedSpeedHoming : public CoordinateWithVelocity
	{
	public:
		AcyclicFixedSpeedHoming
		(
			int Int_InitialCoord,
			int Int_TargetCoord,
			int FixP_Speed
				// must be >= zero
		);
		~AcyclicFixedSpeedHoming();

		virtual ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);

		int GetTarget_Int(void) const;

		void SetTarget_Int
		(
			int Int_TargetCoord
		);
		void SetTarget_FixP
		(
			int FixP_TargetCoord
		);

		void SetSpeed_FixP
		(
			int FixP_Speed
				// must be >= zero
		);

	protected:
		int Int_TargetCoord_Val;
		int FixP_TargetCoord_Val;

		int FixP_Speed_Val;
		
	};
		inline int AcyclicFixedSpeedHoming::GetTarget_Int(void) const {return Int_TargetCoord_Val;}

#endif

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/


/* End of the header ****************************************************/

#endif
