#pragma once
#include "YZDisplayOutput.h"

using namespace std;


class YZDirector3DAdapter
{
	void *m_IDXGIAdapter;
	void *m_AdaptorContext;
	void *m_displayContainer;

	void CreateTexture(int width, int height, int bpp);
public:
	YZDirector3DAdapter();
	~YZDirector3DAdapter();

	int Initialize();

	int DA_FillOutputObjects();

	void DO_Render();

	void DO_SetColor(void *color);

	void UpdateTexture(int width, int height, int bpp, void *texBuf, int plane);

	void D3A_Present();

	void DO_SaveConfig(const char *filename);

	void DO_LoadConfig(const char *filename);
};

