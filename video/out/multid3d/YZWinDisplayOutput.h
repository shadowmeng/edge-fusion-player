#pragma once
#include "YZDisplayOutput.h"
class YZWinDisplayOutput :
	public YZDisplayOutput
{
	void *m_dxOutput;
	void *m_AdaptorContext;
	void *m_WindowContext;

	void *m_criticalSection;


	int m_selectTagIdx;
	bool m_isSelectTag;
	bool m_isEditMode;


	int EdgeIndex(int idx);

	int GetCtlPtCount(int idx);

	void CreateTargetView();
	
	void InitializeVertex();
	void SetVertexBuffer();
	void UpdateVertexBuffer();

	void SetDispalyMatrix();

	void CreateRay(int scrX, int scrY);
	
	void SetCtlPt(float ratio, int ctlIdx);
	void ReSetCtlPt();
	void ReSetVertexWithCtlPt();

	void DrawTag(void *vetex);
	void CreateTagContext();

	bool CheckInPtRect(void *vetex);

	void CreateVertexContext(void *pVertexData, int vertexSize, void *pIndicesData, int indicesSize, void **vertexBuf, void **indicesBuf);

	void SetTriangleIndices(void *buf, int xTileIdx, int yTileIdx);

	void UpdateTexCoord();
public:
	YZWinDisplayOutput();
	YZWinDisplayOutput(void *dxOutput, void *adaptorContext);
	~YZWinDisplayOutput();

	int Initialize(int index);

	void DO_Render();

	void ClickDown();
	void ClickUp();
	void UpdateRay(int scrX, int scrY);

	void UpdateViewMatrix(float z);

	void SwitchEditMode();

	void SetTilesOverlapped(int top, int bottom, int left, int right);

	void DO_SetColor(void *color);

	void DO_UpdateTilePos(int xOffset, int yOffset, int tileTotalWNum, int tileTotalHNum);

	void DO_DrawOnScreen(const TScreenPos *scrPos, int posNum);

	void DO_Present();

	void DO_SetData(void *mem, int size);

	void DO_GetData(void **mem, int *size);

	void DO_RefreshLayout();
};

