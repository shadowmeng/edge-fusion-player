#pragma once

typedef struct tagScreenPos
{
	int x, y;
	float tu, tv;
}TScreenPos;

class YZDisplayOutput
{
	
public:
	YZDisplayOutput();
	//YZDisplayOutput(void *dxOutput);
	virtual ~YZDisplayOutput();

	void DO_SetDXOutputHandle(void *dxOutput);

	virtual int Initialize(int index);
	virtual void DO_Render();
	virtual void UpdateTexture(int width, int height, int bpp, void *texBuf, int plane);
	virtual void DO_SetColor(void *color);
	virtual void SetTilesOverlapped(int top, int bottom, int left, int right);
	virtual void DO_UpdateTilePos(int xOffset, int yOffset, int tileTotalWNum, int tileTotalHNum);
	virtual void DO_DrawOnScreen(const TScreenPos *scrPos, int posNum);
	virtual void DO_Present();
	virtual void DO_SetData(void *mem, int size);
	virtual void DO_GetData(void **mem, int *size);
};

