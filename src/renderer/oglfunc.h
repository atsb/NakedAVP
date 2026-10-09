#ifndef OGLFUNC_H
#define OGLFUNC_H

#if defined(_MSC_VER)
#include <windows.h>
#endif

#if defined(USE_OPENGL_ES)
#include <SDL3/SDL_opengles.h>

// OpenGL compatibility
typedef GLclampf GLclampd;
typedef GLfloat GLdouble;

#else
#include <SDL3/SDL_opengl.h>
#endif

#if !defined(GL_CLAMP_TO_EDGE)
// Originally added by GL_SGIS_texture_edge_clamp; part of OpenGL 1.2 core.
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#if !defined(APIENTRY)
#define APIENTRY
#endif

typedef void (APIENTRY *PFNGLALPHAFUNCPROC)(GLenum, GLclampf);
typedef void (APIENTRY *PFNGLBINDTEXTUREPROC)(GLenum, GLuint);
typedef void (APIENTRY *PFNGLBLENDFUNCPROC)(GLenum, GLenum);
typedef void (APIENTRY *PFNGLCLEARPROC)(GLbitfield);
typedef void (APIENTRY *PFNGLCLEARCOLORPROC)(GLclampf, GLclampf, GLclampf, GLclampf);
typedef void (APIENTRY *PFNGLCOLOR4FPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *PFNGLCOLORPOINTERPROC)(GLint, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *PFNGLCULLFACEPROC)(GLenum);
typedef void (APIENTRY *PFNGLDELETETEXTURESPROC)(GLsizei,const GLuint*);
typedef void (APIENTRY *PFNGLDEPTHFUNCPROC)(GLenum);
typedef void (APIENTRY *PFNGLDEPTHMASKPROC)(GLboolean);
typedef void (APIENTRY *PFNGLDEPTHRANGEPROC)(GLclampd, GLclampd);
typedef void (APIENTRY *PFNGLDISABLEPROC)(GLenum);
typedef void (APIENTRY *PFNGLDISABLECLIENTSTATEPROC)(GLenum);
typedef void (APIENTRY *PFNGLDRAWELEMENTSPROC)(GLenum, GLsizei, GLenum, const GLvoid *);
typedef void (APIENTRY *PFNGLENABLEPROC)(GLenum);
typedef void (APIENTRY *PFNGLENABLECLIENTSTATEPROC)(GLenum);
typedef void (APIENTRY *PFNGLFRONTFACEPROC)(GLenum);
typedef void (APIENTRY *PFNGLGENTEXTURESPROC)(GLsizei,GLuint*);
typedef GLenum (APIENTRY *PFNGLGETERRORPROC)(void);
typedef void (APIENTRY *PFNGLGETFLOATVPROC)(GLenum, GLfloat *);
typedef void (APIENTRY *PFNGLGETINTEGERVPROC)(GLenum, GLint *);
typedef const GLubyte* (APIENTRY *PFNGLGETSTRINGPROC)(GLenum);
typedef void (APIENTRY *PFNGLGETTEXPARAMETERFVPROC)(GLenum, GLenum, GLfloat*);
typedef void (APIENTRY *PFNGLHINTPROC)(GLenum, GLenum);
typedef void (APIENTRY *PFNGLPIXELSTOREIPROC)(GLenum, GLint);
typedef void (APIENTRY *PFNGLPOLYGONOFFSETPROC)(GLfloat, GLfloat);
typedef void (APIENTRY *PFNGLREADPIXELSPROC)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
typedef void (APIENTRY *PFNGLSHADEMODELPROC)(GLenum);
typedef void (APIENTRY *PFNGLTEXCOORDPOINTERPROC)(GLint, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *PFNGLTEXENVFPROC)(GLenum, GLenum, GLfloat);
typedef void (APIENTRY *PFNGLTEXENVFVPROC)(GLenum, GLenum, const GLfloat *);
typedef void (APIENTRY *PFNGLTEXENVIPROC)(GLenum, GLenum, GLint);
typedef void (APIENTRY *PFNGLTEXIMAGE2DPROC)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const GLvoid*);
typedef void (APIENTRY *PFNGLTEXPARAMETERFPROC)(GLenum, GLenum, GLfloat);
typedef void (APIENTRY *PFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
typedef void (APIENTRY *PFNGLTEXSUBIMAGE2DPROC)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const GLvoid*);
typedef void (APIENTRY *PFNGLVERTEXPOINTERPROC)(GLint, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY *PFNGLVIEWPORTPROC)(GLint, GLint, GLsizei, GLsizei);

extern PFNGLALPHAFUNCPROC		pglAlphaFunc;
extern PFNGLBINDTEXTUREPROC		pglBindTexture;
extern PFNGLBLENDFUNCPROC		pglBlendFunc;
extern PFNGLCLEARPROC			pglClear;
extern PFNGLCLEARCOLORPROC		pglClearColor;
extern PFNGLCOLOR4FPROC		pglColor4f;
extern PFNGLCOLORPOINTERPROC		pglColorPointer;
extern PFNGLCULLFACEPROC		pglCullFace;
extern PFNGLDELETETEXTURESPROC		pglDeleteTextures;
extern PFNGLDEPTHFUNCPROC		pglDepthFunc;
extern PFNGLDEPTHMASKPROC		pglDepthMask;
extern PFNGLDEPTHRANGEPROC		pglDepthRange;
extern PFNGLDISABLEPROC		pglDisable;
extern PFNGLDISABLECLIENTSTATEPROC	pglDisableClientState;
extern PFNGLDRAWELEMENTSPROC		pglDrawElements;
extern PFNGLENABLEPROC			pglEnable;
extern PFNGLENABLECLIENTSTATEPROC	pglEnableClientState;
extern PFNGLFRONTFACEPROC		pglFrontFace;
extern PFNGLGENTEXTURESPROC		pglGenTextures;
extern PFNGLGETERRORPROC		pglGetError;
extern PFNGLGETFLOATVPROC		pglGetFloatv;
extern PFNGLGETINTEGERVPROC		pglGetIntegerv;
extern PFNGLGETSTRINGPROC		pglGetString;
extern PFNGLGETTEXPARAMETERFVPROC	pglGetTexParameterfv;
extern PFNGLHINTPROC			pglHint;
extern PFNGLPIXELSTOREIPROC		pglPixelStorei;
extern PFNGLPOLYGONOFFSETPROC		pglPolygonOffset;
extern PFNGLREADPIXELSPROC		pglReadPixels;
extern PFNGLSHADEMODELPROC		pglShadeModel;
extern PFNGLTEXCOORDPOINTERPROC	pglTexCoordPointer;
extern PFNGLTEXENVFPROC		pglTexEnvf;
extern PFNGLTEXENVFVPROC		pglTexEnvfv;
extern PFNGLTEXENVIPROC		pglTexEnvi;
extern PFNGLTEXIMAGE2DPROC		pglTexImage2D;
extern PFNGLTEXPARAMETERFPROC		pglTexParameterf;
extern PFNGLTEXPARAMETERIPROC		pglTexParameteri;
extern PFNGLTEXSUBIMAGE2DPROC		pglTexSubImage2D;
extern PFNGLVERTEXPOINTERPROC		pglVertexPointer;
extern PFNGLVIEWPORTPROC		pglViewport;

extern int ogl_have_multisample_filter_hint;
extern int ogl_have_texture_filter_anisotropic;

extern int ogl_use_multisample_filter_hint;
extern int ogl_use_texture_filter_anisotropic;

extern void load_ogl_functions(int mode);

extern int check_for_errors_(const char *file, int line);
#define check_for_errors() check_for_errors_(__FILE__, __LINE__)

#endif
