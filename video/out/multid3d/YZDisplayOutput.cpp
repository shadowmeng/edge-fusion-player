//#include "stdafx.h"
#include "video/out/multid3d/YZDisplayOutput.h"



YZDisplayOutput::YZDisplayOutput()
{
}


YZDisplayOutput::~YZDisplayOutput()
{
}

void YZDisplayOutput::DO_SetDXOutputHandle(void *dxOutput)
{

}

int YZDisplayOutput::Initialize(int index)
{
	return 0;
}

void YZDisplayOutput::DO_Render()
{

}

void YZDisplayOutput::UpdateTexture(int width, int height, int bpp, void *texBuf, int plane)
{
}

void YZDisplayOutput::DO_SetColor(void *color)
{
}

void YZDisplayOutput::SetTilesOverlapped(int top, int bottom, int left, int right)
{

}

void YZDisplayOutput::DO_UpdateTilePos(int xOffset, int yOffset, int tileTotalWNum, int tileTotalHNum)
{

}

void YZDisplayOutput::DO_DrawOnScreen(const TScreenPos *scrPos, int posNum)
{

}

void YZDisplayOutput::DO_Present()
{

}

void YZDisplayOutput::DO_SetData(void *mem, int size)
{

}

void YZDisplayOutput::DO_GetData(void **mem, int *size)
{

}