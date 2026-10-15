#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include "oglfunc.h"
#include "postprocess.h"

int AVP_BloomEnabled = 1;
static int ppEnvironmentRead;

#if !defined(USE_OPENGL_ES)

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_TEXTURE2
#define GL_TEXTURE2 0x84C2
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif
#ifndef GL_DITHER
#define GL_DITHER 0x0BD0
#endif

typedef GLuint(APIENTRY *AVPCreateShaderProc)(GLenum);
typedef void(APIENTRY *AVPShaderSourceProc)(GLuint, GLsizei, const char *const *, const GLint *);
typedef void(APIENTRY *AVPCompileShaderProc)(GLuint);
typedef void(APIENTRY *AVPGetShaderivProc)(GLuint, GLenum, GLint *);
typedef void(APIENTRY *AVPGetShaderInfoLogProc)(GLuint, GLsizei, GLsizei *, char *);
typedef void(APIENTRY *AVPDeleteShaderProc)(GLuint);
typedef GLuint(APIENTRY *AVPCreateProgramProc)(void);
typedef void(APIENTRY *AVPAttachShaderProc)(GLuint, GLuint);
typedef void(APIENTRY *AVPLinkProgramProc)(GLuint);
typedef void(APIENTRY *AVPGetProgramivProc)(GLuint, GLenum, GLint *);
typedef void(APIENTRY *AVPGetProgramInfoLogProc)(GLuint, GLsizei, GLsizei *, char *);
typedef void(APIENTRY *AVPDeleteProgramProc)(GLuint);
typedef void(APIENTRY *AVPUseProgramProc)(GLuint);
typedef GLint(APIENTRY *AVPGetUniformLocationProc)(GLuint, const char *);
typedef void(APIENTRY *AVPUniform1iProc)(GLint, GLint);
typedef void(APIENTRY *AVPUniform1fProc)(GLint, GLfloat);
typedef void(APIENTRY *AVPUniformMatrix4fvProc)(GLint, GLsizei, GLboolean, const GLfloat *);
typedef void(APIENTRY *AVPActiveTextureProc)(GLenum);
typedef void(APIENTRY *AVPCopyTexSubImage2DProc)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);

typedef struct AVPPostProcessGL
{
    AVPCreateShaderProc CreateShader;
    AVPShaderSourceProc ShaderSource;
    AVPCompileShaderProc CompileShader;
    AVPGetShaderivProc GetShaderiv;
    AVPGetShaderInfoLogProc GetShaderInfoLog;
    AVPDeleteShaderProc DeleteShader;
    AVPCreateProgramProc CreateProgram;
    AVPAttachShaderProc AttachShader;
    AVPLinkProgramProc LinkProgram;
    AVPGetProgramivProc GetProgramiv;
    AVPGetProgramInfoLogProc GetProgramInfoLog;
    AVPDeleteProgramProc DeleteProgram;
    AVPUseProgramProc UseProgram;
    AVPGetUniformLocationProc GetUniformLocation;
    AVPUniform1iProc Uniform1i;
    AVPUniform1fProc Uniform1f;
    AVPUniformMatrix4fvProc UniformMatrix4fv;
    AVPActiveTextureProc ActiveTexture;
    AVPCopyTexSubImage2DProc CopyTexSubImage2D;
} AVPPostProcessGL;

static AVPPostProcessGL glpp;
static int ppAvailable;
static int ppLoggedMissing;
static GLuint sceneTexture;
static GLuint brightTexture;
static GLuint blurTextureA;
static GLuint blurTextureB;
static GLuint meterTexture;
static GLuint exposureTexture;
static int sceneWidth, sceneHeight, effectWidth, effectHeight;
static GLuint brightProgram, blurProgram, compositeProgram;
static GLuint meterProgram, exposureAdaptProgram;
static GLint brightSceneLocation = -1;
static GLint brightExposureLocation = -1;
static GLint brightThresholdLocation = -1;
static GLint brightSoftKneeLocation = -1;
static GLint meterSceneLocation = -1;
static GLint adaptMeterLocation = -1;
static GLint adaptExposureLocation = -1;
static GLint adaptDeltaTimeLocation = -1;
static GLint adaptInitializedLocation = -1;
static GLint blurTextureLocation = -1;
static GLint blurMatrixLocation = -1;
static GLint blurAmountLocation = -1;
static GLint blurXOffsetLocation = -1;
static GLint blurYOffsetLocation = -1;
static GLint compositeSceneLocation = -1;
static GLint compositeBlurLocation = -1;
static GLint compositeExposureLocation = -1;
static GLint compositeStrengthLocation = -1;
static int exposureInitialized;
static Uint64 lastExposureTick;

/* The renderer's state cache and client pointers must be reset after our pass. */
extern void OpenGL_PostProcessResetState(void);

static SDL_FunctionPointer GetGLProc(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

#define LOAD_PP_PROC(member, type, name)     \
    do                                       \
    {                                        \
        glpp.member = (type)GetGLProc(name); \
        if (!glpp.member)                    \
            missing = name;                  \
    } while (0)

static int LoadPostProcessFunctions(void)
{
    const char *missing = NULL;
    memset(&glpp, 0, sizeof(glpp));
    LOAD_PP_PROC(CreateShader, AVPCreateShaderProc, "glCreateShader");
    LOAD_PP_PROC(ShaderSource, AVPShaderSourceProc, "glShaderSource");
    LOAD_PP_PROC(CompileShader, AVPCompileShaderProc, "glCompileShader");
    LOAD_PP_PROC(GetShaderiv, AVPGetShaderivProc, "glGetShaderiv");
    LOAD_PP_PROC(GetShaderInfoLog, AVPGetShaderInfoLogProc, "glGetShaderInfoLog");
    LOAD_PP_PROC(DeleteShader, AVPDeleteShaderProc, "glDeleteShader");
    LOAD_PP_PROC(CreateProgram, AVPCreateProgramProc, "glCreateProgram");
    LOAD_PP_PROC(AttachShader, AVPAttachShaderProc, "glAttachShader");
    LOAD_PP_PROC(LinkProgram, AVPLinkProgramProc, "glLinkProgram");
    LOAD_PP_PROC(GetProgramiv, AVPGetProgramivProc, "glGetProgramiv");
    LOAD_PP_PROC(GetProgramInfoLog, AVPGetProgramInfoLogProc, "glGetProgramInfoLog");
    LOAD_PP_PROC(DeleteProgram, AVPDeleteProgramProc, "glDeleteProgram");
    LOAD_PP_PROC(UseProgram, AVPUseProgramProc, "glUseProgram");
    LOAD_PP_PROC(GetUniformLocation, AVPGetUniformLocationProc, "glGetUniformLocation");
    LOAD_PP_PROC(Uniform1i, AVPUniform1iProc, "glUniform1i");
    LOAD_PP_PROC(Uniform1f, AVPUniform1fProc, "glUniform1f");
    LOAD_PP_PROC(UniformMatrix4fv, AVPUniformMatrix4fvProc, "glUniformMatrix4fv");
    LOAD_PP_PROC(ActiveTexture, AVPActiveTextureProc, "glActiveTexture");
    LOAD_PP_PROC(CopyTexSubImage2D, AVPCopyTexSubImage2DProc, "glCopyTexSubImage2D");

    if (missing)
    {
        if (!ppLoggedMissing)
        {
            fprintf(stderr, "NakedAVP bloom disabled: OpenGL entry point %s is unavailable.\n", missing);
            ppLoggedMissing = 1;
        }
        return 0;
    }
    return 1;
}

static char *ReadShaderFile(const char *name)
{
    char path[2048];
    const char *basePath;
    char *source = NULL;
    FILE *file = NULL;
    long length;

    basePath = SDL_GetBasePath();
    if (basePath)
    {
        if (snprintf(path, sizeof(path), "%sshaders/%s", basePath, name) >= (int)sizeof(path))
            return NULL;
        file = fopen(path, "rb");
    }
    if (!file)
    {
        /* Also permits running directly from a source/build working directory. */
        if (snprintf(path, sizeof(path), "shaders/%s", name) < (int)sizeof(path))
            file = fopen(path, "rb");
    }
    if (!file)
    {
        fprintf(stderr, "NakedAVP bloom: cannot open shader '%s'.\n", name);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return NULL;
    }
    source = (char *)malloc((size_t)length + 1);
    if (!source)
    {
        fclose(file);
        return NULL;
    }
    if (fread(source, 1, (size_t)length, file) != (size_t)length)
    {
        free(source);
        fclose(file);
        return NULL;
    }
    source[length] = '\0';
    fclose(file);
    return source;
}

static GLuint CompileShaderFromFile(GLenum type, const char *filename)
{
    char *source = ReadShaderFile(filename);
    GLuint shader;
    GLint compiled = 0;
    if (!source)
        return 0;

    shader = glpp.CreateShader(type);
    if (!shader)
    {
        free(source);
        return 0;
    }
    glpp.ShaderSource(shader, 1, (const char *const *)&source, NULL);
    glpp.CompileShader(shader);
    glpp.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLint logLength = 0;
        char *log = NULL;
        glpp.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1)
        {
            log = (char *)malloc((size_t)logLength + 1);
            if (log)
            {
                GLsizei written = 0;
                glpp.GetShaderInfoLog(shader, logLength, &written, log);
                log[written >= 0 ? (size_t)written : 0] = '\0';
                fprintf(stderr, "NakedAVP bloom: shader compile failed (%s):\n%s\n", filename, log);
                free(log);
            }
        }
        else
        {
            fprintf(stderr, "NakedAVP bloom: shader compile failed (%s).\n", filename);
        }
        glpp.DeleteShader(shader);
        shader = 0;
    }
    free(source);
    return shader;
}

static GLuint CreateProgramFromFiles(const char *vertexFilename, const char *fragmentFilename)
{
    GLuint vertex = CompileShaderFromFile(GL_VERTEX_SHADER, vertexFilename);
    GLuint fragment = CompileShaderFromFile(GL_FRAGMENT_SHADER, fragmentFilename);
    GLuint program = 0;
    GLint linked = 0;

    if (!vertex || !fragment)
    {
        if (vertex)
            glpp.DeleteShader(vertex);
        if (fragment)
            glpp.DeleteShader(fragment);
        return 0;
    }

    program = glpp.CreateProgram();
    if (program)
    {
        glpp.AttachShader(program, vertex);
        glpp.AttachShader(program, fragment);
        glpp.LinkProgram(program);
        glpp.GetProgramiv(program, GL_LINK_STATUS, &linked);
    }
    glpp.DeleteShader(vertex);
    glpp.DeleteShader(fragment);

    if (!program || !linked)
    {
        GLint logLength = 0;
        char *log = NULL;
        if (program)
            glpp.GetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (program && logLength > 1)
        {
            log = (char *)malloc((size_t)logLength + 1);
            if (log)
            {
                GLsizei written = 0;
                glpp.GetProgramInfoLog(program, logLength, &written, log);
                log[written >= 0 ? (size_t)written : 0] = '\0';
                fprintf(stderr, "NakedAVP bloom: shader link failed (%s + %s):\n%s\n",
                        vertexFilename, fragmentFilename, log);
                free(log);
            }
        }
        else
        {
            fprintf(stderr, "NakedAVP bloom: shader link failed (%s + %s).\n",
                    vertexFilename, fragmentFilename);
        }
        if (program)
            glpp.DeleteProgram(program);
        return 0;
    }
    return program;
}

static void ConfigureTextureFormat(GLuint texture, int width, int height,
                                   GLint internalFormat, GLenum format)
{
    pglBindTexture(GL_TEXTURE_2D, texture);
    pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    pglTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                  format, GL_UNSIGNED_BYTE, NULL);
}

static void ConfigureTexture(GLuint texture, int width, int height)
{
    ConfigureTextureFormat(texture, width, height, GL_RGB, GL_RGB);
}

static int EnsureTextures(int width, int height)
{
    int qWidth = width / 4;
    int qHeight = height / 4;
    int newMeterTexture = 0;
    int newExposureTexture = 0;
    if (qWidth < 1)
        qWidth = 1;
    if (qHeight < 1)
        qHeight = 1;

    if (!sceneTexture)
        pglGenTextures(1, &sceneTexture);
    if (!brightTexture)
        pglGenTextures(1, &brightTexture);
    if (!blurTextureA)
        pglGenTextures(1, &blurTextureA);
    if (!blurTextureB)
        pglGenTextures(1, &blurTextureB);
    if (!meterTexture)
    {
        pglGenTextures(1, &meterTexture);
        newMeterTexture = 1;
    }
    if (!exposureTexture)
    {
        pglGenTextures(1, &exposureTexture);
        newExposureTexture = 1;
    }
    if (!sceneTexture || !brightTexture || !blurTextureA || !blurTextureB ||
        !meterTexture || !exposureTexture)
        return 0;

    if (sceneWidth != width || sceneHeight != height || effectWidth != qWidth || effectHeight != qHeight)
    {
        ConfigureTexture(sceneTexture, width, height);
        ConfigureTexture(brightTexture, qWidth, qHeight);
        ConfigureTexture(blurTextureA, qWidth, qHeight);
        ConfigureTexture(blurTextureB, qWidth, qHeight);
        sceneWidth = width;
        sceneHeight = height;
        effectWidth = qWidth;
        effectHeight = qHeight;
    }
    if (newMeterTexture)
        ConfigureTextureFormat(meterTexture, 1, 1, GL_RGB, GL_RGB);
    if (newExposureTexture)
    {
        ConfigureTexture(exposureTexture, 1, 1);
        exposureInitialized = 0;
        lastExposureTick = 0;
    }
    pglBindTexture(GL_TEXTURE_2D, 0);
    return 1;
}

static void DrawFullscreenQuad(void)
{
    /* Interleaved clip-space xy and normalized uv; OpenGL's origin is bottom-left. */
    static const GLfloat vertices[4 * 4] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};
    static const GLushort indices[6] = {0, 1, 2, 0, 2, 3};

    pglEnableClientState(GL_VERTEX_ARRAY);
    pglVertexPointer(2, GL_FLOAT, (GLsizei)(sizeof(GLfloat) * 4), &vertices[0]);
    pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    pglTexCoordPointer(2, GL_FLOAT, (GLsizei)(sizeof(GLfloat) * 4), &vertices[2]);
    pglDisableClientState(GL_COLOR_ARRAY);
    pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    pglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

static void CopyFramebufferToTexture(GLuint texture, int width, int height)
{
    pglBindTexture(GL_TEXTURE_2D, texture);
    glpp.CopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
}

static float GetExposureDeltaSeconds(void)
{
    Uint64 now = SDL_GetTicks();
    float delta = 1.0f / 60.0f;
    if (lastExposureTick != 0 && now >= lastExposureTick)
        delta = (float)(now - lastExposureTick) / 1000.0f;
    lastExposureTick = now;
    if (delta < 0.001f)
        delta = 0.001f;
    if (delta > 0.10f)
        delta = 0.10f;
    return delta;
}

static void DrawExposureMeter(void)
{
    pglViewport(0, 0, 1, 1);
    glpp.UseProgram(meterProgram);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, sceneTexture);
    if (meterSceneLocation >= 0)
        glpp.Uniform1i(meterSceneLocation, 0);
    DrawFullscreenQuad();
    CopyFramebufferToTexture(meterTexture, 1, 1);
}

static void DrawExposureAdaptation(float deltaSeconds)
{
    pglViewport(0, 0, 1, 1);
    glpp.UseProgram(exposureAdaptProgram);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, meterTexture);
    if (adaptMeterLocation >= 0)
        glpp.Uniform1i(adaptMeterLocation, 0);
    glpp.ActiveTexture(GL_TEXTURE1);
    pglBindTexture(GL_TEXTURE_2D, exposureTexture);
    if (adaptExposureLocation >= 0)
        glpp.Uniform1i(adaptExposureLocation, 1);
    if (adaptDeltaTimeLocation >= 0)
        glpp.Uniform1f(adaptDeltaTimeLocation, deltaSeconds);
    if (adaptInitializedLocation >= 0)
        glpp.Uniform1f(adaptInitializedLocation, exposureInitialized ? 1.0f : 0.0f);
    DrawFullscreenQuad();
    CopyFramebufferToTexture(exposureTexture, 1, 1);
    exposureInitialized = 1;
}

static void DrawBrightPass(void)
{
    glpp.UseProgram(brightProgram);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, sceneTexture);
    if (brightSceneLocation >= 0)
        glpp.Uniform1i(brightSceneLocation, 0);
    glpp.ActiveTexture(GL_TEXTURE1);
    pglBindTexture(GL_TEXTURE_2D, exposureTexture);
    if (brightExposureLocation >= 0)
        glpp.Uniform1i(brightExposureLocation, 1);
    if (brightThresholdLocation >= 0)
        glpp.Uniform1f(brightThresholdLocation, 0.90f);
    if (brightSoftKneeLocation >= 0)
        glpp.Uniform1f(brightSoftKneeLocation, 0.10f);
    DrawFullscreenQuad();
    CopyFramebufferToTexture(brightTexture, effectWidth, effectHeight);
}

static void DrawBlurPass(GLuint sourceTexture, GLuint targetTexture, int horizontal)
{
    static const GLfloat identity[16] = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    glpp.UseProgram(blurProgram);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, sourceTexture);
    if (blurTextureLocation >= 0)
        glpp.Uniform1i(blurTextureLocation, 0);
    if (blurMatrixLocation >= 0)
        glpp.UniformMatrix4fv(blurMatrixLocation, 1, GL_FALSE, identity);
    if (blurAmountLocation >= 0)
        glpp.Uniform1f(blurAmountLocation, 1.70f);
    if (blurXOffsetLocation >= 0)
        glpp.Uniform1f(blurXOffsetLocation, horizontal ? 1.0f / (GLfloat)effectWidth : 0.0f);
    if (blurYOffsetLocation >= 0)
        glpp.Uniform1f(blurYOffsetLocation, horizontal ? 0.0f : 1.0f / (GLfloat)effectHeight);
    DrawFullscreenQuad();
    CopyFramebufferToTexture(targetTexture, effectWidth, effectHeight);
}

static void DrawComposite(void)
{
    glpp.UseProgram(compositeProgram);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, sceneTexture);
    if (compositeSceneLocation >= 0)
        glpp.Uniform1i(compositeSceneLocation, 0);

    glpp.ActiveTexture(GL_TEXTURE1);
    pglBindTexture(GL_TEXTURE_2D, blurTextureB);
    if (compositeBlurLocation >= 0)
        glpp.Uniform1i(compositeBlurLocation, 1);

    glpp.ActiveTexture(GL_TEXTURE2);
    pglBindTexture(GL_TEXTURE_2D, exposureTexture);
    if (compositeExposureLocation >= 0)
        glpp.Uniform1i(compositeExposureLocation, 2);
    if (compositeStrengthLocation >= 0)
        glpp.Uniform1f(compositeStrengthLocation, 0.11f);

    DrawFullscreenQuad();
    glpp.ActiveTexture(GL_TEXTURE2);
    pglBindTexture(GL_TEXTURE_2D, 0);
    pglDisable(GL_TEXTURE_2D);
    glpp.ActiveTexture(GL_TEXTURE1);
    pglBindTexture(GL_TEXTURE_2D, 0);
    pglDisable(GL_TEXTURE_2D);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, 0);
    glpp.UseProgram(0);
}

#endif /* !USE_OPENGL_ES */

void AVP_PostProcess_Init(void)
{
#if !defined(USE_OPENGL_ES)
    if (!ppEnvironmentRead)
    {
        const char *disable = getenv("NAKEDAVP_BLOOM");
        ppEnvironmentRead = 1;
        if (disable && (strcmp(disable, "0") == 0 || strcmp(disable, "off") == 0 ||
                        strcmp(disable, "false") == 0 || strcmp(disable, "OFF") == 0 ||
                        strcmp(disable, "FALSE") == 0))
        {
            /* This supplies the initial default only; the in-game menu can re-enable bloom. */
            AVP_BloomEnabled = 0;
            fprintf(stderr, "NakedAVP bloom defaulted off by NAKEDAVP_BLOOM.\n");
        }
    }
    if (ppAvailable)
        return;
    if (!LoadPostProcessFunctions())
        return;

    brightProgram = CreateProgramFromFiles("PostEffect_Screen_vp.vert", "PostEffect_BrightPass_fp.frag");
    /* These are the supplied HPL blur shaders, used as shipped. */
    blurProgram = CreateProgramFromFiles("PostEffect_Blur_vp.vert", "PostEffect_Blur_2D_fp.frag");
    compositeProgram = CreateProgramFromFiles("PostEffect_Screen_vp.vert", "PostEffect_Bloom_NakedAVP_fp.frag");
    meterProgram = CreateProgramFromFiles("PostEffect_Screen_vp.vert", "PostEffect_ExposureMeter_fp.frag");
    exposureAdaptProgram = CreateProgramFromFiles("PostEffect_Screen_vp.vert", "PostEffect_ExposureAdapt_fp.frag");
    if (!brightProgram || !blurProgram || !compositeProgram || !meterProgram || !exposureAdaptProgram)
    {
        if (brightProgram)
            glpp.DeleteProgram(brightProgram);
        if (blurProgram)
            glpp.DeleteProgram(blurProgram);
        if (compositeProgram)
            glpp.DeleteProgram(compositeProgram);
        if (meterProgram)
            glpp.DeleteProgram(meterProgram);
        if (exposureAdaptProgram)
            glpp.DeleteProgram(exposureAdaptProgram);
        brightProgram = blurProgram = compositeProgram = meterProgram = exposureAdaptProgram = 0;
        ppAvailable = 0;
        fprintf(stderr, "NakedAVP adaptive bloom disabled because one or more shader programs failed to initialise.\n");
        return;
    }
    brightSceneLocation = glpp.GetUniformLocation(brightProgram, "sceneTex");
    brightExposureLocation = glpp.GetUniformLocation(brightProgram, "exposureTex");
    brightThresholdLocation = glpp.GetUniformLocation(brightProgram, "threshold");
    brightSoftKneeLocation = glpp.GetUniformLocation(brightProgram, "softKnee");
    meterSceneLocation = glpp.GetUniformLocation(meterProgram, "sceneTex");
    adaptMeterLocation = glpp.GetUniformLocation(exposureAdaptProgram, "meterTex");
    adaptExposureLocation = glpp.GetUniformLocation(exposureAdaptProgram, "exposureTex");
    adaptDeltaTimeLocation = glpp.GetUniformLocation(exposureAdaptProgram, "deltaTime");
    adaptInitializedLocation = glpp.GetUniformLocation(exposureAdaptProgram, "exposureInitialized");
    blurTextureLocation = glpp.GetUniformLocation(blurProgram, "textureU0");
    blurMatrixLocation = glpp.GetUniformLocation(blurProgram, "worldViewProj");
    blurAmountLocation = glpp.GetUniformLocation(blurProgram, "amount");
    blurXOffsetLocation = glpp.GetUniformLocation(blurProgram, "xOffset");
    blurYOffsetLocation = glpp.GetUniformLocation(blurProgram, "yOffset");
    compositeSceneLocation = glpp.GetUniformLocation(compositeProgram, "sceneTex");
    compositeBlurLocation = glpp.GetUniformLocation(compositeProgram, "blurTex");
    compositeExposureLocation = glpp.GetUniformLocation(compositeProgram, "exposureTex");
    compositeStrengthLocation = glpp.GetUniformLocation(compositeProgram, "bloomStrength");

    ppAvailable = 1;
    exposureInitialized = 0;
    lastExposureTick = 0;
    fprintf(stderr, "NakedAVP adaptive bloom enabled (gaze-weighted exposure, soft highlight shoulder, reduced glow).\n");
#endif
}

void AVP_PostProcess_Render(int width, int height)
{
#if !defined(USE_OPENGL_ES)
    GLint oldViewport[4] = {0, 0, width, height};
    if (!AVP_BloomEnabled || !ppAvailable || width <= 0 || height <= 0)
        return;
    glpp.ActiveTexture(GL_TEXTURE0);
    if (!EnsureTextures(width, height))
        return;

    pglGetIntegerv(GL_VIEWPORT, oldViewport);
    /* The source is already drawn into the default framebuffer by the engine. */
    glpp.ActiveTexture(GL_TEXTURE0);
    CopyFramebufferToTexture(sceneTexture, width, height);

    pglDisable(GL_ALPHA_TEST);
    pglDisable(GL_BLEND);
    pglDisable(GL_DEPTH_TEST);
    pglDepthMask(GL_FALSE);

    pglDisable(GL_DITHER);
    DrawExposureMeter();
    DrawExposureAdaptation(GetExposureDeltaSeconds());
    pglEnable(GL_DITHER);

    pglViewport(0, 0, effectWidth, effectHeight);
    DrawBrightPass();
    DrawBlurPass(brightTexture, blurTextureA, 1);
    DrawBlurPass(blurTextureA, blurTextureB, 0);

    pglViewport(0, 0, width, height);
    DrawComposite();

    /* Restore core renderer assumptions and invalidate its cached bindings. */
    glpp.ActiveTexture(GL_TEXTURE2);
    pglBindTexture(GL_TEXTURE_2D, 0);
    pglDisable(GL_TEXTURE_2D);
    glpp.ActiveTexture(GL_TEXTURE1);
    pglBindTexture(GL_TEXTURE_2D, 0);
    pglDisable(GL_TEXTURE_2D);
    glpp.ActiveTexture(GL_TEXTURE0);
    pglBindTexture(GL_TEXTURE_2D, 0);
    pglEnable(GL_TEXTURE_2D);
    pglEnable(GL_ALPHA_TEST);
    pglAlphaFunc(GL_GREATER, 0.0f);
    pglEnable(GL_BLEND);
    pglBlendFunc(GL_SRC_ALPHA, GL_ONE);
    pglEnable(GL_DEPTH_TEST);
    pglDepthFunc(GL_LEQUAL);
    pglDepthMask(GL_TRUE);
    pglViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    glpp.UseProgram(0);
    OpenGL_PostProcessResetState();
#endif
    (void)width;
    (void)height;
}

void AVP_PostProcess_Shutdown(void)
{
#if !defined(USE_OPENGL_ES)
    if (ppAvailable || sceneTexture || brightTexture || blurTextureA || blurTextureB ||
        meterTexture || exposureTexture)
    {
        if (sceneTexture)
            pglDeleteTextures(1, &sceneTexture);
        if (brightTexture)
            pglDeleteTextures(1, &brightTexture);
        if (blurTextureA)
            pglDeleteTextures(1, &blurTextureA);
        if (blurTextureB)
            pglDeleteTextures(1, &blurTextureB);
        if (meterTexture)
            pglDeleteTextures(1, &meterTexture);
        if (exposureTexture)
            pglDeleteTextures(1, &exposureTexture);
        sceneTexture = brightTexture = blurTextureA = blurTextureB = 0;
        meterTexture = exposureTexture = 0;
        sceneWidth = sceneHeight = effectWidth = effectHeight = 0;
    }
    if (brightProgram && glpp.DeleteProgram)
        glpp.DeleteProgram(brightProgram);
    if (blurProgram && glpp.DeleteProgram)
        glpp.DeleteProgram(blurProgram);
    if (compositeProgram && glpp.DeleteProgram)
        glpp.DeleteProgram(compositeProgram);
    if (meterProgram && glpp.DeleteProgram)
        glpp.DeleteProgram(meterProgram);
    if (exposureAdaptProgram && glpp.DeleteProgram)
        glpp.DeleteProgram(exposureAdaptProgram);
    brightProgram = blurProgram = compositeProgram = meterProgram = exposureAdaptProgram = 0;
    brightSceneLocation = brightExposureLocation = brightThresholdLocation = brightSoftKneeLocation = -1;
    meterSceneLocation = -1;
    adaptMeterLocation = adaptExposureLocation = adaptDeltaTimeLocation = adaptInitializedLocation = -1;
    blurTextureLocation = blurMatrixLocation = blurAmountLocation = -1;
    blurXOffsetLocation = blurYOffsetLocation = -1;
    compositeSceneLocation = compositeBlurLocation = compositeExposureLocation = compositeStrengthLocation = -1;
    exposureInitialized = 0;
    lastExposureTick = 0;
    ppAvailable = 0;
#endif
}
