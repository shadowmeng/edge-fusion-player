#ifndef MULTID3D_API_H
#define MULTID3D_API_H


#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int multid3d_initialize();

__declspec(dllexport) void multi3d_draw();

__declspec(dllexport) void multi3d_flip_page();

__declspec(dllexport) void multi3d_copyimg(int w, int h, int bpp, void *buf, int plane);

__declspec(dllexport) void multi3d_setcolormatrix(void *color);

__declspec(dllexport) void multi3d_saveFile(const char *filename);

__declspec(dllexport) void multi3d_loadFile(const char *filename);

#ifdef __cplusplus
}
#endif

#endif