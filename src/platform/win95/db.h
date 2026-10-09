/* ********************************************************************	*
 *																		*
 *	DB.H - Header for debugging functions and macros.					*
 *																		*
 *	By: Garry Lancaster									Version: 2.0	*
 *																		*
 * ******************************************************************** */

/* N O T E S ********************************************************** */

/* Define NDEBUG here to switch off all debugging. */

/* Set the DB_LEVEL here or before this file is included. Most db macros
 * have a level from 1 to 5. They
 * will only compile to code if the DB_LEVEL is at or greater than their
 * level, otherwise they will be translated to ((void) 0) (i.e. no
 * code.). The levels should be used as follows:
 *
 *	1 - Very low cost debugging. Negligible speed penalty. Could easily
 *		be left in a finished game.
 *	2 - Low cost debugging. Small speed penalty. Use during development
 *		for well-tested code.
 *	3 - Medium cost debugging. Obvious but tolerable speed penalty. Use
 *		during development most of the time.
 *	4 - High cost debugging. Large speed penalty. Use during development
 *		when actively bug hunting.
 *	5 - Very high cost debugging. Massive speed penalty. Use when trying
 *		to track down one of THOSE bugs.
 *
 * The level of a macro is part of its name e.g. to code a db_assert
 * that fires at level 3 or above, use db_assert3().
 */

/* If you do not set the DB_LEVEL, it is set for you: to 3 */
#ifndef DB_LEVEL
	#define DB_LEVEL 3
#endif

/* N.B. If NDEBUG is set, it over-rides DB_LEVEL and switches off all
 * debugging.
 */
#ifdef NDEBUG
	#undef DB_LEVEL
	#define DB_LEVEL	0
#endif

/* Some db macros can be made optional dependent on the setting of the
 * global variable db_option by appending _opt e.g. db_assert_opt(). The
 * only code that is executed for any _opt macro if db_option is zero is
 *
 * 	if(db_option)
 *
 * However, this is still more than the macros controlled by the DB_LEVEL
 * - if they are above the current DB_LEVEL they cause no code execution
 * whatsoever. Therefore, avoid using _opt type macros inside extremely
 * time critical code - use macros contolled by the DB_LEVEL instead,
 * unless you are prepared to put up with the speed penalty.
 * The only time that _opt type macros generate no code is when NDEBUG is
 * defined.
 */

/* S T A R T   W R A P P E R ****************************************** */

/* Avoid multiple inclusions of this file in a single source file. */
#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

/* I N C L U D E D S ************************************************** */
//#include "advwin32.h"

/* Permit use in a C++ source file. */
#ifdef __cplusplus
extern "C" {
#endif

/* ******************************************************************** *
 * 																		*
 *	I N T E R F A C E - both internal and external.						*
 * 																		*
 * ******************************************************************** */

/* C O N S T A N T S **************************************************	*/

/* Possible values for the global variable, db_display_type. */
#define DB_DOS			0
#define DB_WINDOWS		1
#define DB_DIRECTDRAW	2

/* Possible values for the bltOrFlip field of db_dd_mode_tag structure. */
#define DB_FLIP	0
#define DB_BLT	1

/* M A C R O S ********************************************************	*/

#if (!defined(DB_NOASSUME)) && defined(_MSC_VER) && (_MSC_VER >= 1200)
	#define _db_assume(x) __assume(x)
#else
	#define _db_assume(x) ((void)0)
#endif

#if DB_LEVEL >= 1
	/* Fn like macro. */
	#define db_set_mode(md, miP) db_set_mode_ex(md, miP, NULL)
	#define db_set_log_file(strP) db_set_log_file_ex(strP)
#else
	#define db_set_mode(md, miP) ((void) 0)
	#define db_set_log_file(strP) ((void) 0)
#endif

/* Final use macros after testing of DB_LEVEL / db_option. */
#define db_assert_final(expr) \
	((expr) ? ((void) 0) : db_assert_fail(#expr, __FILE__, __LINE__))
	
/* Macros whose compilation is conditional on the value of DB_LEVEL. */
#if DB_LEVEL >= 1
	#define db_assert1(expr)            db_assert_final(expr)
	#define db_onlyassert1(expr)        db_assert_final(expr)
	#define db_verify1(expr)            db_assert_final(expr)
	#define db_print1(x, y, strP)       db_print_fired(x, y, strP)
	#define db_msg1(strP)               db_msg_fired(strP)
	#define db_log1(strP)               db_log_fired(strP)
	#define db_code1(code)              code
	#define db_printf1(params)          db_printf_fired params
	#define db_msgf1(params)            db_msgf_fired params
	#define db_logf1(params)            db_logf_fired params
#else
	#define db_assert1(expr)            _db_assume(expr)
	#define db_onlyassert1(__ignore)    ((void) 0)
	#define db_verify1(expr)            (expr)
	#define db_print1(x, y, __ignore)   ((void) 0)
	#define db_msg1(__ignore)           _db_assume(0)
	#define db_log1(__ignore)           ((void) 0)
	#define db_code1(__ignore)		    
	#define db_printf1(x, y, __ignore)  ((void) 0)
	#define db_msgf1(__ignore)          _db_assume(0)
	#define db_logf1(__ignore)          ((void) 0)
#endif
#if DB_LEVEL >= 2
	#define db_assert2(expr)            db_assert_final(expr)
	#define db_onlyassert2(expr)        db_assert_final(expr)
	#define db_verify2(expr)            db_assert_final(expr)
	#define db_print2(x, y, strP)       db_print_fired(x, y, strP)
	#define db_msg2(strP)               db_msg_fired(strP)
	#define db_log2(strP)               db_log_fired(strP)
	#define db_code2(code)              code
	#define db_printf2(params)          db_printf_fired params
	#define db_msgf2(params)            db_msgf_fired params
	#define db_logf2(params)            db_logf_fired params
#else
	#define db_assert2(expr)            _db_assume(expr)
	#define db_onlyassert2(__ignore)    ((void) 0)
	#define db_verify2(expr)            (expr)
	#define db_print2(x, y, __ignore)   ((void) 0)
	#define db_msg2(__ignore)           _db_assume(0)
	#define db_log2(__ignore)           ((void) 0)
	#define db_code2(__ignore)		    
	#define db_printf2(x, y, __ignore)  ((void) 0)
	#define db_msgf2(__ignore)          _db_assume(0)
	#define db_logf2(__ignore)          ((void) 0)
#endif
#if DB_LEVEL >= 3
	#define db_assert3(expr)            db_assert_final(expr)
	#define db_onlyassert3(expr)        db_assert_final(expr)
	#define db_verify3(expr)            db_assert_final(expr)
	#define db_print3(x, y, strP)       db_print_fired(x, y, strP)
	#define db_msg3(strP)               db_msg_fired(strP)
	#define db_log3(strP)               db_log_fired(strP)
	#define db_code3(code)              code
	#define db_printf3(params)          db_printf_fired params
	#define db_msgf3(params)            db_msgf_fired params
	#define db_logf3(params)            db_logf_fired params
#else
	#define db_assert3(expr)            _db_assume(expr)
	#define db_onlyassert3(__ignore)    ((void) 0)
	#define db_verify3(expr)            (expr)
	#define db_print3(x, y, __ignore)   ((void) 0)
	#define db_msg3(__ignore)           _db_assume(0)
	#define db_log3(__ignore)           ((void) 0)
	#define db_code3(__ignore)		    
	#define db_printf3(x, y, __ignore)  ((void) 0)
	#define db_msgf3(__ignore)          _db_assume(0)
	#define db_logf3(__ignore)          ((void) 0)
#endif
#if DB_LEVEL >= 4
	#define db_assert4(expr)            db_assert_final(expr)
	#define db_onlyassert4(expr)        db_assert_final(expr)
	#define db_verify4(expr)            db_assert_final(expr)
	#define db_print4(x, y, strP)       db_print_fired(x, y, strP)
	#define db_msg4(strP)               db_msg_fired(strP)
	#define db_log4(strP)               db_log_fired(strP)
	#define db_code4(code)              code
	#define db_printf4(params)          db_printf_fired params
	#define db_msgf4(params)            db_msgf_fired params
	#define db_logf4(params)            db_logf_fired params
#else
	#define db_assert4(expr)            _db_assume(expr)
	#define db_onlyassert4(__ignore)    ((void) 0)
	#define db_verify4(expr)            (expr)
	#define db_print4(x, y, __ignore)   ((void) 0)
	#define db_msg4(__ignore)           _db_assume(0)
	#define db_log4(__ignore)           ((void) 0)
	#define db_code4(__ignore)		    
	#define db_printf4(x, y, __ignore)  ((void) 0)
	#define db_msgf4(__ignore)          _db_assume(0)
	#define db_logf4(__ignore)          ((void) 0)
#endif
#if DB_LEVEL >= 5
	#define db_assert5(expr)            db_assert_final(expr)
	#define db_onlyassert5(expr)        db_assert_final(expr)
	#define db_verify5(expr)            db_assert_final(expr)
	#define db_print5(x, y, strP)       db_print_fired(x, y, strP)
	#define db_msg5(strP)               db_msg_fired(strP)
	#define db_log5(strP)               db_log_fired(strP)
	#define db_code5(code)              code
	#define db_printf5(params)          db_printf_fired params
	#define db_msgf5(params)            db_msgf_fired params
	#define db_logf5(params)            db_logf_fired params
#else
	#define db_assert5(expr)            _db_assume(expr)
	#define db_onlyassert5(__ignore)    ((void) 0)
	#define db_verify5(expr)            (expr)
	#define db_print5(x, y, __ignore)   ((void) 0)
	#define db_msg5(__ignore)           _db_assume(0)
	#define db_log5(__ignore)           ((void) 0)
	#define db_code5(__ignore)		    
	#define db_printf5(x, y, __ignore)  ((void) 0)
	#define db_msgf5(__ignore)          _db_assume(0)
	#define db_logf5(__ignore)          ((void) 0)
#endif

/* Macros which fire if db_option is non-zero (and NDEBUG is not
 * defined).
 */
#ifndef NDEBUG
	#define db_assert_opt(expr)         if(db_option) db_assert_final(expr)
	#define db_onlyassert_opt(expr)     if(db_option) db_assert_final(expr)
	#define db_verify_opt(expr)         ((db_option) ? db_assert_final(expr) : (expr))
	#define db_print_opt(x, y, strP)    if(db_option) db_print_fired(x, y, strP)
	#define db_msg_opt(strP)            if(db_option) db_msg_fired(strP)
	#define db_log_opt(strP)            if(db_option) db_log_fired(strP)
	#define db_code_opt(code)           if(db_option) code
	#define db_printf_opt(params)       if(db_option) db_printf_fired params
	#define db_msgf_opt(params)         if(db_option) db_msgf_fired params
	#define db_logf_opt(params)         if(db_option) db_logf_fired params
#else
	#define db_assert_opt(expr)         _db_assume(expr)
	#define db_onlyassert_opt(__ignore) ((void) 0)
	#define db_verify_opt(expr)         (expr)
	#define db_print_opt(x,y,__ignore)  ((void) 0)
	#define db_msg_opt(__ignore)        _db_assume(0)
	#define db_log_opt(__ignore)        ((void) 0)
	#define db_code_opt(code)           
	#define db_printf_opt(params)       ((void) 0)
	#define db_msgf_opt(params)         _db_assume(0)
	#define db_logf_opt(params)         ((void) 0)			
#endif

/* Macros for setting and getting db_option. */
#ifndef NDEBUG
	#define db_option_set(status)	db_option = (status)
	#define db_option_on()			db_option = 1
	#define db_option_off()			db_option = 0
	#define db_option_get()			(db_option)
#else
	#define db_option_set(__ignore)		((void) 0)
	#define db_option_on()				((void) 0)
	#define db_option_off()				((void) 0)
	#define db_option_get()				(0)
#endif

/* T Y P E S ********************************************************** */

struct db_dd_mode_tag
{
	void *directDrawP;
	void *visibleSurfaceP;
	void *drawSurfaceP;
	int width, height, bitsPerPixel;
	unsigned short foreCol, backCol;
	int bltOrFlip;
	int bltXOffset, bltYOffset;
};

/* P R O T O S ******************************************************** */

/* Don't prototype anything or declare globals if NDEBUG is defined. */
#ifndef NDEBUG

#define __cdecl

/* New formatted debugging fns. */
extern void __cdecl db_logf_fired(const char *fmtStrP, ...);
extern void __cdecl db_printf_fired(int x, int y, const char *fmtStrP, ...);
extern void __cdecl db_msgf_fired(const char *fmtStrP, ...);

/* Called whenever an assertion fails. */
extern void db_assert_fail(const char *exprP, const char *fileP, int line);

/* Displays a message and has the program pause until the user responds
 * to it.
 */
extern void db_msg_fired(const char *strP);

/* Displays a message (on platforms that support positioning, at (x, y)) 
 * and continues program execution immediately. 
 */
extern void db_print_fired(int x, int y, const char *strP);

/* Writes a message to a log file. */
extern void db_log_fired(const char *strP);

/* Deletes the old log file, so that the log file only contains messages
 * saved from this point on. Use ONCE at the start of any program that
 * uses any of the db_log macros.
 */
extern void db_log_init(void);

/* Gets the current infomation needed for the display mode. Used to enable
 * other code to use the same debugging stuff. The return value is modeInfoP
 */
extern int db_get_mode(void **modeInfoPP, void **FontPP);

/* Changes the log file name */
extern void db_set_log_file_ex(const char *strP);

/* Sets the display mode for the debugging functions to use. mode must
 * be one of DB_DOS, DB_WINDOWS or DB_DIRECTDRAW. The modeInfoP parameter
 * is NULL except for Direct Draw. The Last parameter can be NULL.
 */
extern void db_set_mode_ex(int mode, void *modeInfoP, void *newFontP);

/* Called to set whether exceptions or brakepoints are called. */
extern void DbUseBrakepoints(BOOL use_brakepoints);

/* Call this to de-allocate memory used to store the debugging font. This
 * fn does nothing unless you are in DirectDraw mode, since this is the 
 * only mode which loads its own font. Calling this fn is not strictly
 * necessary since the OS will de-allocate a process' outstanding dynamic
 * memory allocations when it ends anyway. However, calling this fn is 
 * cleaner and avoids BoundsChecker hits.
 */
extern void db_uninit(void);

 /* G L O B A L S ******************************************************	*/

/* Should we expand _opt type macros? */
extern int db_option;

#endif /* of #ifndef NDEBUG */

/* E N D   W R A P P E R ********************************************** */

/* Permit use in a C++ source file. */
#ifdef __cplusplus
}
#endif

/* Avoid multiple inclusions of this file in a single source file. */
#endif
