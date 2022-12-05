/*
	
	r2base.h

	Created 13/11/97 by David Malcolm

	"R2" is a proposed 2d-rendering interface; this
	is the base header for it
	
*/

#ifndef _r2base
#define _r2base 1

//#ifdef __cplusplus
	//extern "C" {
//#endif

/* Version settings *****************************************************/
	#define UseTemplates	No

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	struct r2pos
	{
		int x;
		int y;

		#ifdef __cplusplus
		static const r2pos Origin;
			// always (0,0)

		// Construction
		r2pos() : x(0), y(0)
		{
			// empty
		}
		r2pos(int x_new,int y_new) : x(x_new), y(y_new)
		{
			// empty
		}

		#if 0
		friend bool operator==(const r2pos& R2Pos_1, const r2pos& R2Pos_2);
		#endif

		int bIsOrigin(void) const;

		friend r2pos operator+ ( const r2pos& R2Pos_1, const r2pos& R2Pos_2	);
		void operator+= ( const r2pos& R2Pos );
			// standard vector arithmetic

		r2pos FixP_Scale
		(
			int FixP_ScaleFactor
		) const;
			// assumes the position to be in 16:16 fixed point,
			// returns the position scaled by the fixed pt factor

		int ApproxMagnitude(void) const;
			// assumes the position to be in 16:16 fixed point

		#endif // __cplusplus		
	}; // suggested naming convention: "R2Pos"
	
	#ifdef __cplusplus
		// Inline methods:
		inline int r2pos::bIsOrigin(void) const
		{
			if (x==0)
			{
				if (y==0)
				{
					return Yes;
				}
			}
			return No;
		}

		inline void r2pos::operator+=
		(
			const r2pos& R2Pos
		)
		{
			x += R2Pos . x;
			y += R2Pos . y;
		}

		inline int r2pos::ApproxMagnitude(void) const
		{
			// assumes the position to be in 16:16 fixed point
			// based around the Foley+vanDam 2d distance function
			int xAbs = (x>0) ? x : -x;
			int yAbs = (y>0) ? y : -y;

			int xyMax = (xAbs > yAbs) ? xAbs : yAbs;

			return (xAbs + yAbs + (xyMax * 2));
		}
	#endif /* __cplusplus */


	#if UseTemplates
		#ifdef __cplusplus
		// C++ implementation:
		// 		r2size defined from a template
		template <class ValType> struct size2d
		{
			ValType w;
			ValType h;

		// Construction:
			size2d() : w(0), h(0)
			{
				// empty
			}

			size2d(ValType w_new,ValType h_new) : w(w_new), h(h_new)
			{
				// empty
			}

		// Method declarations:
			int bHasArea(void);
				// Does this size have non-zero area?:

			void VCompose(const size2d& Size2D_Other );
			void HCompose(const size2d& Size2D_Other );
				// update size to the size necessary for composing
				// this and another size either vertically or horizontally


		// Inline methods:
			int bHasArea(void)
			{
				// Does this size have non-zero area?:
				if (w<=0)
				{
					return No;
				}
				if (h<=0)
				{
					return No;
				}
				return Yes;
			}
			void VCompose(const size2d& Size2D_Other )
			{
				// sum of heights; greater of widths
				
				h += Size2D_Other . h;

				if (w < Size2D_Other . w)
				{
					w = Size2D_Other . w;
				}
			}
			void HCompose(const size2d& Size2D_Other )
			{
				// sum of widths; greater of heights
				w += Size2D_Other . w;

				if (h < Size2D_Other . h)
				{
					h = Size2D_Other . h;
				}
			}

		};  // Suggested naming convention: "Size2D"
		
		typedef size2d<int> r2size; // suggested naming convention: "R2Size"
		#else
		/* C implementation: r2size defined as a simple struct: */

		struct r2size
		{
			int w;
			int h;
		}; /* suggested naming convention: "R2Size" */

		#endif	// __cplusplus
	#else	// UseTemplates
	struct r2size
	{
		int w;
		int h;

	#ifdef __cplusplus
	// Construction:
		r2size() : w(0), h(0)
		{
			// empty
		}

		r2size(int w_new,int h_new) : w(w_new), h(h_new)
		{
			// empty
		}

	// Method declarations:
		int bHasArea(void);
			// Does this size have non-zero area?:

		void VCompose(const r2size& R2Size_Other );
		void HCompose(const r2size& R2Size_Other );
			// update size to the size necessary for composing
			// this and another size either vertically or horizontally

	#endif // __cplusplus

	};  // Suggested naming convention: "Size2D"

	#ifdef __cplusplus
	// Inline methods:
		inline int r2size::bHasArea(void)
		{
			// Does this size have non-zero area?:
			if (w<=0)
			{
				return No;
			}
			if (h<=0)
			{
				return No;
			}
			return Yes;
		}
		inline void r2size::VCompose(const r2size& R2Size_Other )
		{
			// sum of heights; greater of widths
			
			h += R2Size_Other . h;

			if (w < R2Size_Other . w)
			{
				w = R2Size_Other . w;
			}
		}
		inline void r2size::HCompose(const r2size& R2Size_Other )
		{
			// sum of widths; greater of heights
			w += R2Size_Other . w;

			if (h < R2Size_Other . h)
			{
				h = R2Size_Other . h;
			}
		}
	#endif /* __cplusplus */
		
	#endif	// UseTemplates

	struct r2rect
	{
		int x0;
		int y0;
		int x1;
		int y1;

		// Rectangle defined in the "Windows" fashion
		// i.e. first coord is top-left of rectangle and inside it
		// second coord is bottom-right of rectange and immediately
		// outside it.  If the second coord is co-incident with the first
		// or to the left or above, then the rectangle has zero area; no
		// points are within it

	#ifdef __cplusplus
	// Friends:
		friend void R2BASE_ScreenModeChange_Cleanup(void);
			// allowed to update the internal record of the dimensions of the physical screen

	// Construction:
		r2rect(int x0_new,int y0_new,int x1_new,int y1_new) :
			x0(x0_new),
			y0(y0_new),
			x1(x1_new),
			y1(y1_new)
		{
			// empty
		}

		r2rect(int x0_new,int y0_new, r2size R2Size) :
			x0(x0_new),
			y0(y0_new),
			x1(x0_new + R2Size . w),
			y1(y0_new + R2Size . h)
		{
			// empty
		}

		r2rect(struct r2pos R2Pos, r2size R2Size) :
			x0(R2Pos . x),
			y0(R2Pos . y),
			x1(R2Pos . x + R2Size . w),
			y1(R2Pos . y + R2Size . h)
		{
			// empty
		}

		r2rect(struct r2pos R2Pos, int W,int H) :
			x0(R2Pos . x),
			y0(R2Pos . y),
			x1(R2Pos . x + W),
			y1(R2Pos . y + H)
		{
			// empty
		}

		// Method declarations:
		int bHasArea(void) const;
		struct r2pos GetPos(void) const;
		r2size GetSize(void) const;
		int Width(void) const;
		int Height(void) const;
		void SetWidth(int w_New);
		void SetHeight(int h_New);
		int bWithin( const struct r2pos& R2Pos ) const;
			// is point inside this rect

		int bFitsIn( const struct r2rect& R2Rect ) const;
			// is this rect fully inside ?

		int bOverlap( const struct r2rect& R2Rect_ToTest ) const;
		void Clip( struct r2rect& R2Rect_ToClip ) const;
		void Clip(const struct r2rect& R2Rect_In, struct r2rect& R2Rect_Out ) const;
		static const r2rect& PhysicalScreen(void);
			// for read-only access to the dimensions of the physical screen

		int bValidPhys(void) const;
			// is this a valid rect within the physical screen?
			// useful for asserting in a rendering routine

		// Access to 9 "interesting" positions in the rectangle:
		// top-left, top-middle, etc...
		r2pos Hotspot_TL(void) const;
		r2pos Hotspot_TM(void) const;
		r2pos Hotspot_TR(void) const;
		r2pos Hotspot_ML(void) const;
		r2pos Hotspot_MM(void) const;
		r2pos Hotspot_MR(void) const;
		r2pos Hotspot_BL(void) const;
		r2pos Hotspot_BM(void) const;
		r2pos Hotspot_BR(void) const;

		r2pos Centre(void) const { return Hotspot_MM(); }

		r2pos CentredWithSize_TL( r2size R2Size_In ) const;
			// finds location for top left of a rect with the given size so that
			// it gets centred on this rect

	// Rendering:
		void AlphaFill
		(
			unsigned char R,
			unsigned char G,
			unsigned char B,
			unsigned char translucency
		) const;


	private:
		static r2rect R2Rect_PhysicalScreen;

	#endif // __cplusplus
	}; // suggested naming convention: "R2Rect"
	
	#ifdef __cplusplus
	// Inline methods:
		inline int r2rect::bHasArea(void) const
		{
			if (x1<=x0)
			{
				return No;
			}
			if (y1<=y0)
			{
				return No;
			}
			return Yes;
		}
		inline struct r2pos r2rect::GetPos(void) const
		{
			return r2pos(x0,y0);
		}
		inline r2size r2rect::GetSize(void) const
		{
			return r2size(x1-x0,y1-y0);
		}
		inline int r2rect::Width(void) const
		{
			return x1-x0;
		}
		inline int r2rect::Height(void) const
		{
			return y1-y0;
		}
		inline void r2rect::SetWidth(int w_New)
		{
			x1 = x0 + w_New;
		}
		inline void r2rect::SetHeight(int h_New)
		{
			y1 = y0 + h_New;
		}
		inline int r2rect::bWithin( const struct r2pos& R2Pos ) const
		{
			if
			(
				R2Pos . x < x0
			)
			{
				return No;
			}
			if
			(
				R2Pos . y < y0
			)
			{
				return No;
			}
			if
			(
				R2Pos . x >= x1
			)
			{
				return No;
			}
			if
			(
				R2Pos . y >= y1
			)
			{
				return No;
			}
			return Yes;
		}
		inline int r2rect::bFitsIn( const struct r2rect& R2Rect ) const
		{
			if ( x0 < R2Rect . x0)
			{
				return No;
			}
			if ( y0 < R2Rect . y0)
			{
				return No;
			}
			if ( x1 > R2Rect . x1)
			{
				return No;
			}
			if ( y1 > R2Rect . y1)
			{
				return No;
			}
			return Yes;
		}
		inline int r2rect::bOverlap( const struct r2rect& R2Rect_ToTest ) const
		{
			if ( x0 >= R2Rect_ToTest . x1 )
			{
				return No;
			}
			if ( y0 >= R2Rect_ToTest . y1 )
			{
				return No;
			}
			if ( x1 <= R2Rect_ToTest . x0 )
			{
				return No;
			}
			if ( y1 <= R2Rect_ToTest . y0 )
			{
				return No;
			}
			return Yes;
		}
		inline void r2rect::Clip( struct r2rect& R2Rect_ToClip ) const
		{
			if ( R2Rect_ToClip . x0 < x0)
			{
				R2Rect_ToClip . x0 = x0;
			}
			if ( R2Rect_ToClip . y0 < y0)
			{
				R2Rect_ToClip . y0 = y0;
			}
			if ( R2Rect_ToClip . x1 >= x1)
			{
				R2Rect_ToClip . x1 = x1;
			}
			if ( R2Rect_ToClip . y1 >= y1)
			{
				R2Rect_ToClip . y1 = y1;
			}
		}
		#if 0
		inline void r2rect::Clip(const struct r2rect& R2Rect_In, struct r2rect& R2Rect_Out )
		{
			R2Rect_Out . x0 = 
		}
		#endif
		inline /*static*/ const r2rect& r2rect::PhysicalScreen(void)
		{
			return R2Rect_PhysicalScreen;
		}
		inline int r2rect::bValidPhys(void) const
		{
			// is this a valid rect within the physical screen?
			// useful for asserting in a rendering routine
			if ( x0 < 0 )
			{
				return No;
			}
			if ( y0 < 0 )
			{
				return No;
			}
			if ( x1 > R2Rect_PhysicalScreen . x1 )
			{
				return No;
			}
			if ( y1 > R2Rect_PhysicalScreen . y1 )
			{
				return No;
			}
			// Check for well-formedness:
			if ( x0 > x1 )
			{
				return No;
			}
			if ( y0 > y1 )
			{
				return No;
			}

			return Yes;
		}
		inline r2pos r2rect::Hotspot_TL(void) const
		{
			return r2pos
			(				
				x0,
				y0
			);
		}
		inline r2pos r2rect::Hotspot_TM(void) const
		{
			return r2pos
			(
				(x0+x1)/2,
				y0
			);
		}
		inline r2pos r2rect::Hotspot_TR(void) const
		{
			return r2pos
			(
				(x1-1),
				y0
			);
		}
		inline r2pos r2rect::Hotspot_ML(void) const
		{
			return r2pos
			(
				x0,
				(y0+y1)/2
			);
		}
		inline r2pos r2rect::Hotspot_MM(void) const
		{
			return r2pos
			(
				(x0+x1)/2,
				(y0+y1)/2
			);
		}
		inline r2pos r2rect::Hotspot_MR(void) const
		{
			return r2pos
			(
				(x1-1),
				(y0+y1)/2
			);
		}
		inline r2pos r2rect::Hotspot_BL(void) const
		{
			return r2pos
			(
				x0,
				(y1-1)
			);
		}
		inline r2pos r2rect::Hotspot_BM(void) const
		{
			return r2pos
			(
				(x0+x1)/2,
				(y1-1)
			);
		}
		inline r2pos r2rect::Hotspot_BR(void) const
		{
			return r2pos
			(
				(x1-1),
				(y1-1)
			);
		}

		inline r2pos r2rect::CentredWithSize_TL( r2size R2Size_In ) const
		{
			return r2pos
			(
				x0+((Width() - R2Size_In . w)/2),
				y0+((Height() - R2Size_In . h)/2)
			);
		}
	#endif /* __cplusplus */

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	extern void R2BASE_ScreenModeChange_Setup(void);
	extern void R2BASE_ScreenModeChange_Cleanup(void);


/* End of the header ****************************************************/


//#ifdef __cplusplus
//	};
//#endif

#endif
