#ifndef NAKEDAVP_POSTPROCESS_H
#define NAKEDAVP_POSTPROCESS_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern int AVP_BloomEnabled;
    void AVP_PostProcess_Init(void);
    void AVP_PostProcess_Render(int width, int height);
    void AVP_PostProcess_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
