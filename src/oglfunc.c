#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"

#include "oglfunc.h"

PFNGLALPHAFUNCPROC		pglAlphaFunc;
PFNGLBINDTEXTUREPROC		pglBindTexture;
PFNGLBLENDFUNCPROC		pglBlendFunc;
PFNGLCLEARPROC			pglClear;
PFNGLCLEARCOLORPROC		pglClearColor;
PFNGLCOLOR4FPROC		pglColor4f;
PFNGLCOLORPOINTERPROC		pglColorPointer;
PFNGLCULLFACEPROC		pglCullFace;
PFNGLDELETETEXTURESPROC		pglDeleteTextures;
PFNGLDEPTHFUNCPROC		pglDepthFunc;
PFNGLDEPTHMASKPROC		pglDepthMask;
PFNGLDEPTHRANGEPROC		pglDepthRange;
PFNGLDISABLEPROC		pglDisable;
PFNGLDISABLECLIENTSTATEPROC	pglDisableClientState;
PFNGLDRAWELEMENTSPROC		pglDrawElements;
PFNGLENABLEPROC			pglEnable;
PFNGLENABLECLIENTSTATEPROC	pglEnableClientState;
PFNGLFRONTFACEPROC		pglFrontFace;
PFNGLGENTEXTURESPROC		pglGenTextures;
PFNGLGETERRORPROC		pglGetError;
PFNGLGETFLOATVPROC		pglGetFloatv;
PFNGLGETINTEGERVPROC		pglGetIntegerv;
PFNGLGETSTRINGPROC		pglGetString;
PFNGLGETTEXPARAMETERFVPROC	pglGetTexParameterfv;
PFNGLHINTPROC			pglHint;
PFNGLPIXELSTOREIPROC		pglPixelStorei;
PFNGLPOLYGONOFFSETPROC		pglPolygonOffset;
PFNGLREADPIXELSPROC		pglReadPixels;
PFNGLSHADEMODELPROC		pglShadeModel;
PFNGLTEXCOORDPOINTERPROC	pglTexCoordPointer;
PFNGLTEXENVFPROC		pglTexEnvf;
PFNGLTEXENVFVPROC		pglTexEnvfv;
PFNGLTEXENVIPROC		pglTexEnvi;
PFNGLTEXIMAGE2DPROC		pglTexImage2D;
PFNGLTEXPARAMETERFPROC		pglTexParameterf;
PFNGLTEXPARAMETERIPROC		pglTexParameteri;
PFNGLTEXSUBIMAGE2DPROC		pglTexSubImage2D;
PFNGLVERTEXPOINTERPROC		pglVertexPointer;
PFNGLVIEWPORTPROC		pglViewport;

int ogl_have_multisample_filter_hint;
int ogl_have_texture_filter_anisotropic;

int ogl_use_multisample_filter_hint;
int ogl_use_texture_filter_anisotropic;

static void dummyfunc()
{
}

#define LoadOGLProc_(type, func, name) {                    \
	if (!mode) p##func = (type) dummyfunc; else			\
	p##func = (type) SDL_GL_GetProcAddress(#name);			\
	if (p##func == NULL) {						\
		if (!ogl_missing_func) ogl_missing_func = #func;	\
	}								\
}

#define LoadOGLProc(type, func)						\
	LoadOGLProc_(type, func, func)

#define LoadOGLProc2(type, func1, func2)					\
	LoadOGLProc_(type, func1, func1); \
	if (p##func1 == NULL) { \
		ogl_missing_func = NULL; \
		LoadOGLProc_(type, func1, func2); \
	}

static int check_token(const char *string, const char *token)
{
	const char *s = string;
	int len = strlen(token);
	
	while ((s = strstr(s, token)) != NULL) {
		const char *next = s + len;
		
		if ((s == string || *(s-1) == ' ') &&
			(*next == 0 || *next == ' ')) {
			
			return 1;
		}
		
		s = next;
	}
	
	return 0;
}

void load_ogl_functions(int mode)
{
	const char* ogl_missing_func;
	const char* ext;

	ogl_missing_func = NULL;
	
	LoadOGLProc(PFNGLALPHAFUNCPROC, glAlphaFunc);
	LoadOGLProc(PFNGLBINDTEXTUREPROC, glBindTexture);
	LoadOGLProc(PFNGLBLENDFUNCPROC, glBlendFunc);
	LoadOGLProc(PFNGLCLEARPROC, glClear);
	LoadOGLProc(PFNGLCLEARCOLORPROC, glClearColor);
	LoadOGLProc(PFNGLCOLOR4FPROC, glColor4f);
	LoadOGLProc(PFNGLCOLORPOINTERPROC, glColorPointer);
	LoadOGLProc(PFNGLCULLFACEPROC, glCullFace);
	LoadOGLProc(PFNGLDELETETEXTURESPROC, glDeleteTextures);
	LoadOGLProc(PFNGLDEPTHFUNCPROC, glDepthFunc);
	LoadOGLProc(PFNGLDEPTHMASKPROC, glDepthMask);
	LoadOGLProc2(PFNGLDEPTHRANGEPROC, glDepthRange, glDepthRangef);
	LoadOGLProc(PFNGLDISABLEPROC, glDisable);
	LoadOGLProc(PFNGLDISABLECLIENTSTATEPROC, glDisableClientState);
	LoadOGLProc(PFNGLDRAWELEMENTSPROC, glDrawElements);
	LoadOGLProc(PFNGLENABLEPROC, glEnable);
	LoadOGLProc(PFNGLENABLECLIENTSTATEPROC, glEnableClientState);
	LoadOGLProc(PFNGLFRONTFACEPROC, glFrontFace);
	LoadOGLProc(PFNGLGENTEXTURESPROC, glGenTextures);
	LoadOGLProc(PFNGLGETERRORPROC, glGetError);
	LoadOGLProc(PFNGLGETFLOATVPROC, glGetFloatv);
	LoadOGLProc(PFNGLGETINTEGERVPROC, glGetIntegerv);
	LoadOGLProc(PFNGLGETSTRINGPROC, glGetString);
	LoadOGLProc(PFNGLGETTEXPARAMETERFVPROC, glGetTexParameterfv);
	LoadOGLProc(PFNGLHINTPROC, glHint);
	LoadOGLProc(PFNGLPIXELSTOREIPROC, glPixelStorei);
	LoadOGLProc(PFNGLPOLYGONOFFSETPROC, glPolygonOffset);
	LoadOGLProc(PFNGLREADPIXELSPROC, glReadPixels);
	LoadOGLProc(PFNGLSHADEMODELPROC, glShadeModel);
	LoadOGLProc(PFNGLTEXCOORDPOINTERPROC, glTexCoordPointer);
	LoadOGLProc(PFNGLTEXENVFPROC, glTexEnvf);
	LoadOGLProc(PFNGLTEXENVFVPROC, glTexEnvfv);
	LoadOGLProc(PFNGLTEXENVIPROC, glTexEnvi);
	LoadOGLProc(PFNGLTEXIMAGE2DPROC, glTexImage2D);
	LoadOGLProc(PFNGLTEXPARAMETERFPROC, glTexParameterf);
	LoadOGLProc(PFNGLTEXPARAMETERIPROC, glTexParameteri);
	LoadOGLProc(PFNGLTEXSUBIMAGE2DPROC, glTexSubImage2D);
	LoadOGLProc(PFNGLVERTEXPOINTERPROC, glVertexPointer);
	LoadOGLProc(PFNGLVIEWPORTPROC, glViewport);

	if (!mode) {
		return;
	}
	
	if (ogl_missing_func) {
		fprintf(stderr, "Unable to load OpenGL Library: missing function %s\n", ogl_missing_func);
		exit(EXIT_FAILURE);
	}
	
#if !defined(NDEBUG)
	printf("GL_VENDOR: %s\n", pglGetString(GL_VENDOR));
	printf("GL_RENDERER: %s\n", pglGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", pglGetString(GL_VERSION));
	//printf("GL_SHADING_LANGUAGE_VERSION: %s\n", pglGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("GL_EXTENSIONS: %s\n", pglGetString(GL_EXTENSIONS));
#endif

	ext = (const char *) pglGetString(GL_EXTENSIONS);

	ogl_have_multisample_filter_hint = check_token(ext, "GL_NV_multisample_filter_hint");
	ogl_have_texture_filter_anisotropic = check_token(ext, "GL_EXT_texture_filter_anisotropic");

	ogl_use_multisample_filter_hint = ogl_have_multisample_filter_hint;
	ogl_use_texture_filter_anisotropic = ogl_have_texture_filter_anisotropic;
}

int check_for_errors_(const char *file, int line)
{
	GLenum error;
	int diderror = 0;
	
	while ((error = pglGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OPENGL ERROR: %04X (%s:%d)\n", error, file, line);
		
		diderror = 1;
	}
	
	return diderror;
}
