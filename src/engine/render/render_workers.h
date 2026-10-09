#ifndef AVP_RENDER_WORKERS_H
#define AVP_RENDER_WORKERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AVP_GLVertex {
    float v[4];
    float t[2];
    uint8_t c[4];
    uint8_t s[4];
} AVP_GLVertex;

typedef enum AVP_RenderVertexMode {
    AVP_RENDER_VERTEX_GOURAUD_TEXTURED,
    AVP_RENDER_VERTEX_GOURAUD,
    AVP_RENDER_VERTEX_CLOAKED,
    AVP_RENDER_VERTEX_DECAL,
    AVP_RENDER_VERTEX_PARTICLE,
    AVP_RENDER_VERTEX_THERMAL,
    AVP_RENDER_VERTEX_SKY
} AVP_RenderVertexMode;

typedef struct AVP_RenderVertexJobParams {
    float z_near;
    float proj_x;
    float proj_y;
    float centre_x;
    float centre_y;
    float recip_w;
    float recip_h;
    float hud_z_offset;
    int color_r;
    int color_g;
    int color_b;
    int color_a;
    int flags;
} AVP_RenderVertexJobParams;

void AVP_RenderWorkers_Init(void);
void AVP_RenderWorkers_Shutdown(void);
void AVP_RenderWorkers_Submit(const void *vertices, size_t count,
                              AVP_GLVertex *output,
                              AVP_RenderVertexMode mode,
                              const AVP_RenderVertexJobParams *params);
void AVP_RenderWorkers_WaitAll(void);
int AVP_RenderWorkers_IsEnabled(void);

#ifdef __cplusplus
}
#endif

#endif
