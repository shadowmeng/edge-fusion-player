//#include "stdafx.h"
#include "YZWinDisplayOutput.h"
#include "YUDirect3DCommon.h"
#include <Windows.h>
#include <stddef.h>

//#define SHOW_ERROR()
#define SHOW_ERROR() do{ \
	char szErrorLine[20]; \
	sprintf_s(szErrorLine, sizeof(szErrorLine), "line:%d", __LINE__); \
	MessageBox(nullptr, __FILE__, szErrorLine, MB_OK); \
} while(0)


#ifdef USE_D3D9_MATH
#include <assert.h>
#include <d3dx9math.h>
#include <d3d10shader.h>
//#include <D3DX10Async.h>
const float XM_PIDIV2 = 1.570796327f;

#define XMFLOAT3 D3DXVECTOR3
#define XMFLOAT2 D3DXVECTOR2
#define XMMATRIX D3DXMATRIX
#define XMVECTOR D3DXVECTOR4 
#define XMFLOAT4 D3DXVECTOR4
#define XMVectorSet D3DXVECTOR4

#define VECTOR3(v) XMFLOAT3(v.x, v.y, v.z)
#define MJSMatrixLookAtLH(r,e,a,u) do { \
	XMFLOAT3 v[3] = { VECTOR3(e), VECTOR3(a), VECTOR3(u) }; \
	D3DXMatrixLookAtLH(&r, &v[0], &v[1], &v[2]); \
}while(0)

#define MJSMatrixIdentity(x) do {D3DXMatrixIdentity(&x);}while(0)
#define MJSMatrixTranspose(a,b) do {D3DXMatrixTranspose(&a, &b);}while(0)

#define MJSMatrixInverse(a,b) do {D3DXMatrixInverse(&a, NULL, &b);}while(0)

#define MJSVector4Transform(a,b) do {D3DXVec4Transform(&a, &a, &b);}while(0)

#define MJSMatrixPerspectiveFovLH(a,b,c,d,e) do {D3DXMatrixPerspectiveFovLH(&a, b,c,d,e);}while(0)

inline void XMStoreFloat4(XMFLOAT4 *left, XMVECTOR ret)
{
	(*left).x = ret.x;
	(*left).y = ret.y;
	(*left).z = ret.z;
	(*left).w = ret.w;
}

inline XMVECTOR XMLoadFloat4(XMFLOAT4 *left)
{
	XMVECTOR ret;
	ret.x = (*left).x;
	ret.y = (*left).y;
	ret.z = (*left).z;
	ret.w = (*left).w;
	return ret;
}

inline void XMStoreFloat3(XMFLOAT3 *left, XMVECTOR ret)
{
	(*left).x = ret.x;
	(*left).y = ret.y;
	(*left).z = ret.z;
}

inline XMVECTOR XMLoadFloat3(XMFLOAT3 *left)
{
	XMVECTOR ret;
	ret.x = (*left).x;
	ret.y = (*left).y;
	ret.z = (*left).z;
	return ret;
}

inline void XMStoreFloat2(XMFLOAT2 *left, XMVECTOR ret)
{
	(*left).x = ret.x;
	(*left).y = ret.y;
}

inline XMVECTOR XMLoadFloat2(XMFLOAT2 *left)
{
	XMVECTOR ret;
	ret.x = (*left).x;
	ret.y = (*left).y;
	return ret;
}

bool Vector2NearEqual(XMFLOAT3 a, XMFLOAT3 b, float v)
{
	if (abs(a.x - b.x) < v
		&&abs(a.y - b.y) < v)
		return TRUE;

	return FALSE;
}

inline XMFLOAT3 D3DXVec2Cross(XMFLOAT3 &a, XMFLOAT3 &b)
{
	float w = a.x * b.y - a.y * b.x;
	XMFLOAT3 ret = XMFLOAT3(w, w, w);
	return ret;
}

inline XMFLOAT3 XMVector2IntersectLineXX(XMFLOAT3 startYPt, XMFLOAT3 endYPt,
	XMFLOAT3 startXPt, XMFLOAT3 endXPt)
{
	XMFLOAT3 ret;
	XMFLOAT3 a, b, c, C1, C2;
	int v[] = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };
	XMFLOAT3 g_XMQNaN = XMFLOAT3((float)v[0], (float)v[1], (float)v[2]);
	int f[] = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
	XMFLOAT3 g_XMInfinity = XMFLOAT3((float)f[0], (float)f[1], (float)f[2]);

	a = (endYPt)-(startYPt);
	b = (endXPt)-(startXPt);
	c = (startYPt)-(startXPt);

	C1 = D3DXVec2Cross(a, b);
	C2 = D3DXVec2Cross(b, c);

	if (Vector2NearEqual(C1, XMFLOAT3(0, 0, 0), 0.0001)){
		if (Vector2NearEqual(C2, XMFLOAT3(0, 0, 0), 0.0001)){
			return g_XMInfinity;
		}
		else
		{
			return g_XMQNaN;
		}
	}
	else{
		ret = startYPt + XMFLOAT3(C1.x == 0 ? 0 : a.x * C2.x / C1.x, 
			C1.y ==0 ? 0 :a.y * C2.y / C1.y, 
			C1.z == 0 ? 0 : a.z * C2.z / C1.z);
	}

	return ret;
}

inline XMVECTOR XMVector2IntersectLine(XMVECTOR &a, XMVECTOR &b, XMVECTOR &c, XMVECTOR &d)
{
	XMFLOAT3 ret = XMVector2IntersectLineXX(VECTOR3(a), VECTOR3(b), VECTOR3(c), VECTOR3(d));
	return XMVECTOR(ret.x, ret.y, ret.z, 1);
}

extern "C"
{
extern HRESULT WINAPI D3DX10CompileFromFileA(LPCSTR pSrcFile,CONST D3D10_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,
        LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void* pPump, ID3D10Blob** ppShader, ID3D10Blob** ppErrorMsgs, HRESULT* pHResult);
		
extern HRESULT WINAPI
D3DCompileFromFile(_In_ LPCSTR pFileName,
                   _In_reads_opt_(_Inexpressible_(pDefines->Name != NULL)) CONST D3D_SHADER_MACRO* pDefines,
                   _In_opt_ ID3DInclude* pInclude,
                   _In_ LPCSTR pEntrypoint,
                   _In_ LPCSTR pTarget,
                   _In_ UINT Flags1,
                   _In_ UINT Flags2,
                   _Out_ ID3DBlob** ppCode,
                   _Out_opt_ ID3DBlob** ppErrorMsgs);

extern HRESULT WINAPI
D3DCompile(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
_In_ SIZE_T SrcDataSize,
_In_opt_ LPCSTR pSourceName,
_In_reads_opt_(_Inexpressible_(pDefines->Name != NULL)) CONST D3D_SHADER_MACRO* pDefines,
_In_opt_ ID3DInclude* pInclude,
_In_opt_ LPCSTR pEntrypoint,
_In_ LPCSTR pTarget,
_In_ UINT Flags1,
_In_ UINT Flags2,
_Out_ ID3DBlob** ppCode,
_Out_opt_ ID3DBlob** ppErrorMsgs);
}
#else

#include <d3d10.h>
#include <d3d10_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

using namespace DirectX;

#define MJSMatrixLookAtLH(r,e,a,u) do {r = XMMatrixLookAtLH(e,a,u);}while(0)
#define MJSMatrixIdentity(x) do {x = XMMatrixIdentity();}while(0)
#define MJSMatrixTranspose(a,b) do {a = XMMatrixTranspose(b);}while(0)
#define MJSMatrixInverse(a,b) do {a = XMMatrixInverse(NULL, b);}while(0)
#define MJSVector4Transform(a,b) do {a = XMVector4Transform(a, b);}while(0)
#define MJSMatrixPerspectiveFovLH(a,b,c,d,e) do {a = XMMatrixPerspectiveFovLH(b,c,d,e);}while(0)
#endif

const char *glsl =
#include <video/out/multid3d/render.h>
;

//#define CTL_NUM 4
#define CTL_HORIZONTAL_NUM (window->wSubTileNum - 1)
#define CTL_VERTICAL_NUM (window->hSubTileNum - 1)

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)    ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)    ((int)(short)HIWORD(lParam))
#endif

enum ECtlPtIndex
{
    CTL_LT=0,
    CTL_LB=1,
    CTL_RT=2,
    CTL_RB=3,
    CTL_T1=4,
 /*
 CTL_T2=5,

 CTL_B1=6,
 CTL_B2=7,

 CTL_L1=8,
 CTL_L2=9,

 CTL_R1=10,
CTL_R2=11,
CTL_MAX = 12*/
};

//#define CTL_MAX (CTL_T1 + 4 * CTL_NUM)
#define CTL_MAX (CTL_T1 + 2 * (CTL_HORIZONTAL_NUM + CTL_VERTICAL_NUM))
enum ECtlMidPtIndex
{
	CTL_MID_TOP = 0,
	CTL_MID_BOTTOM = 1,
	CTL_MID_LEFT = 2,
	CTL_MID_RIGHT = 3
};

struct TPtIndexEntry
{
	int startPtIndex;
	int endPtIndex;
};

struct OverLappedEdgeBuffer
{
	XMFLOAT4 overLappedEdge;
	XMFLOAT4 textCoord;
};

TPtIndexEntry g_ptIndexEntries[] = {
		{ CTL_LT, CTL_RT },
		{ CTL_LB, CTL_RB },
		{ CTL_LB, CTL_LT },
		{ CTL_RB, CTL_RT },
};

#define IN_CTLPT_RANGE(x) (x >= CTL_T1 && x < CTL_MAX)



#define CTL_START_PT(idx) g_ptIndexEntries[EdgeIndex(idx)].startPtIndex
#define CTL_END_PT(idx) g_ptIndexEntries[EdgeIndex(idx)].endPtIndex
#define IDX_START_PT(idx) g_ptIndexEntries[idx].startPtIndex
#define IDX_END_PT(idx) g_ptIndexEntries[idx].endPtIndex
#define CHECK_HORIZONTAL(idx)  ((idx - CTL_T1)< CTL_HORIZONTAL_NUM * 2 )


struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

enum paramType
{
	PT_OBJECT,
	PT_ARRAY,
	PT_VAR_ARRAY,
};

union MemberSize
{
	int value;
	int offset;
	int (*GetSizeFun)(void *);
};

union MemberCount
{
	int value;
	int offset;
	int(*GetSizeFun)(void *);
};

struct AggregateProp
{
	char *szName;
	paramType type;
	int offset;
	MemberSize memSize;
	MemberCount memCount;
};

#define MEMBER_SIZE(ctype, m) sizeof((((ctype *)0)->m))
#define ELEMENT_SIZE(ctype, m) sizeof((((ctype *)0)->m[0]))

#define OPT_AP_ENTRY(ctype, memtype, optname, member) \
{optname, memtype, offsetof(ctype, member), MEMBER_SIZE(ctype, member), 1}

#define OPT_AP_ENTRY_ARRAY(ctype, memtype, optname, member, count) \
{optname, memtype, offsetof(ctype, member), ELEMENT_SIZE(ctype, member), count}

#define OPT_AP_ENTRY_VAR_ARRAY(ctype, memtype, optname, member, varible) \
{optname, memtype, offsetof(ctype, member), ELEMENT_SIZE(ctype, member), offsetof(ctype, varible)}

#define OPT_PROP(memtype, name) \
	OPT_AP_ENTRY(WindowDataContainer, memtype, #name, name)

#define OPT_PROP_ARRAY(memtype, name, count) \
	OPT_AP_ENTRY_ARRAY(WindowDataContainer, memtype, #name, name, count)

#define OPT_PROP_VAR_ARRAY(memtype, name, varible) \
	OPT_AP_ENTRY_VAR_ARRAY(WindowDataContainer, memtype, #name, name, varible)


struct WindowDataContainer
{
	WindowDataContainer(){
		swapChain = NULL;
		renderTargetView = NULL;
		depthStencilView = NULL;

		vertexBuffer = NULL;
		indicesBuffer = NULL;
		constantBuffer = NULL;

		pVertexShader = NULL;
		pPixelShader = NULL;

		tagVertexBuffer = NULL;
		tagIndicesBuffer = NULL;

		vertexMem = NULL;

		planeIndexBuffer = NULL;


		memset(&g_World, 0x0, sizeof(g_World));
		memset(&g_Projection, 0x0, sizeof(g_Projection));
		memset(&g_View, 0x0, sizeof(g_View));

		viewZPos = -0.1f;

		//planeMem = NULL;

		texCoord[CTL_LB] = XMFLOAT2(0.0, 1.0);
		texCoord[CTL_RB] = XMFLOAT2(1.0, 1.0);
		texCoord[CTL_RT] = XMFLOAT2(1.0, 0.0);
		texCoord[CTL_LT] = XMFLOAT2(0.0, 0.0);
	}

	XMMATRIX                g_World;
	XMMATRIX                g_View;
	XMMATRIX                g_Projection;

	XMVECTOR RayStartPos;
	XMVECTOR RayDir;

	XMFLOAT3 RayPos[2];

	//Direct3D 10 stuff per window data
	IDXGISwapChain* swapChain;
	ID3D10RenderTargetView* renderTargetView;
	ID3D10DepthStencilView* depthStencilView;

	ID3D10Buffer *vertexBuffer;
	ID3D10Buffer *rayBuffer;
	ID3D10Buffer *rayIndicesBuffer;
	ID3D10Buffer *indicesBuffer;
	ID3D10Buffer *constantBuffer;

	ID3D10Buffer *planeIndexBuffer;

	ID3D10Buffer *tagVertexBuffer;
	ID3D10Buffer *tagIndicesBuffer;

	ID3D10VertexShader *pVertexShader;
	ID3D10PixelShader *pPixelShader;
	ID3D10PixelShader *pPixelShader2;

	// window goodies
	HWND hWnd;
	int width;
	int height;

	DWORD vertexCount;
	DWORD indicesCount;
	DWORD planeIndicesCount;

	DWORD controlCount;
	XMFLOAT3 *controlVertex;
	float *controlRatio;

	XMFLOAT2 texCoord[4];

	SimpleVertex *vertexMem;
	DWORD *indicesMem;
	DWORD *planeIndicesMem;

	XMFLOAT3 lastClickPos;

	int wSubTileNum;
	int hSubTileNum;

	float viewZPos;

	//DWORD *planeMem;
	
	XMMATRIX colormatrix;
	ID3D10Buffer *colorBuffer;

	OverLappedEdgeBuffer overLappedEdge;
	ID3D10Buffer *overLappedEdgeBuffer;

	int leftOverlapped;
	int rightOverlapped;
	int topOverlapped;
	int bottomOverlapped;

	int m_xTileOffset;
	int m_yTileOffset;
	int m_tileTotalWNum;
	int m_tileTotalHNum;
};

AggregateProp optionProp[] = {
	OPT_PROP(PT_OBJECT, g_World),
	OPT_PROP(PT_OBJECT, g_View),
	OPT_PROP(PT_OBJECT ,g_Projection),
	OPT_PROP(PT_OBJECT, RayStartPos),
	OPT_PROP(PT_OBJECT, RayDir),
	OPT_PROP(PT_OBJECT, RayPos),
	OPT_PROP(PT_OBJECT, width),
	OPT_PROP(PT_OBJECT, height),
	OPT_PROP(PT_OBJECT, vertexCount),
	OPT_PROP(PT_OBJECT, indicesCount),
	OPT_PROP(PT_OBJECT, planeIndicesCount),
	OPT_PROP(PT_OBJECT, controlCount),
	OPT_PROP_VAR_ARRAY(PT_VAR_ARRAY, controlVertex, controlCount),
	OPT_PROP_VAR_ARRAY(PT_VAR_ARRAY, controlRatio, controlCount),
	OPT_PROP(PT_OBJECT, texCoord),
	OPT_PROP_VAR_ARRAY(PT_VAR_ARRAY, vertexMem, vertexCount),
	OPT_PROP_VAR_ARRAY(PT_VAR_ARRAY, indicesMem, indicesCount),
	OPT_PROP_VAR_ARRAY(PT_VAR_ARRAY, planeIndicesMem, planeIndicesCount),
	OPT_PROP(PT_OBJECT, wSubTileNum),
	OPT_PROP(PT_OBJECT, hSubTileNum),
	OPT_PROP(PT_OBJECT, viewZPos),
	OPT_PROP(PT_OBJECT, leftOverlapped),
	OPT_PROP(PT_OBJECT, rightOverlapped),
	OPT_PROP(PT_OBJECT, topOverlapped),
	OPT_PROP(PT_OBJECT, bottomOverlapped),
	OPT_PROP(PT_OBJECT, m_xTileOffset),
	OPT_PROP(PT_OBJECT, m_yTileOffset),
	OPT_PROP(PT_OBJECT, m_tileTotalWNum),
	OPT_PROP(PT_OBJECT, m_tileTotalHNum),
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 mPaintColor;
};

class WDOLock
{
	CRITICAL_SECTION cs;
public:
	WDOLock(){
		InitializeCriticalSection(&cs);
	}

	~WDOLock(){
		DeleteCriticalSection(&cs);
	}

	void Lock(){
		EnterCriticalSection(&cs);
	}

	void UnLock(){
		LeaveCriticalSection(&cs);
	}
};


class CSync
{
	WDOLock *m_lock;
public:
	CSync(void *lock){
		m_lock = (WDOLock*)lock;
		m_lock->Lock();
	}

	~CSync(){
		m_lock->UnLock();
	}
};

inline static int GetMemberSize(AggregateProp *prop, void *object)
{
	int memSize = 0;
	unsigned char *baseAddr = (unsigned char *)object;
	switch (prop->type)
	{
	case PT_OBJECT:
		memSize = prop->memSize.value;
		break;
	case PT_VAR_ARRAY:
		memSize = prop->memSize.value * (*(DWORD*)(baseAddr + prop->memCount.offset));
		break;
	default:
		assert(false);
		break;
	}
	return memSize;
}

inline static void SetMemberValue(AggregateProp *prop, void *object, void *mem, int *size)
{
	unsigned char *baseAddr = (unsigned char *)object;
	*size = GetMemberSize(prop, object);
	switch (prop->type)
	{
	case PT_OBJECT:
		memcpy(baseAddr + prop->offset, mem, *size);
		break;
	case PT_VAR_ARRAY:
	{
		void *addr = (void *)*(unsigned long*)(baseAddr + prop->offset);
		if (*size > GetMemberSize(prop, object)){
			delete [](unsigned char *)addr;
			addr = new unsigned char[*size];
			memcpy(baseAddr + prop->offset, &addr, sizeof(void*));
		}
		memcpy(addr, mem, *size);
		break;
	}
	default:
		assert(false);
		break;
	}
}

inline static void GetMemberValue(AggregateProp *prop, void *object, void *mem, int *size)
{
	unsigned char *baseAddr = (unsigned char *)object;
	*size = GetMemberSize(prop, object);
	switch (prop->type)
	{
	case PT_OBJECT:
		memcpy(mem, baseAddr + prop->offset, *size);
		break;
	case PT_VAR_ARRAY:
	{
		void *addr = (void *)*(unsigned long*)(baseAddr + prop->offset);
		memcpy(mem, addr, *size);
		break;
	}
	default:
		assert(false);
		break;
	}
}

inline static int GetAggregateSize(AggregateProp *prop, int count, void *object)
{
	int totalSize = 0;
	while (count > 0){
		totalSize += GetMemberSize(prop, object);
		prop += 1;
		count--;
	}
	return totalSize;
}

inline static bool Serilize(void *object, void **outMem, int *outSize)
{
	WindowDataContainer *windowData = (WindowDataContainer*)object;
	int idx, memCount = sizeof(optionProp) / sizeof(optionProp[0]);
	int blockSize = GetAggregateSize(optionProp, memCount, object);

	unsigned char *outBuf = new unsigned char[blockSize + sizeof(DWORD)];
	unsigned char *curBuf = outBuf;

	memcpy(curBuf, &blockSize, sizeof(DWORD));
	curBuf += sizeof(DWORD);
	idx = 0;
	while (idx < memCount)
	{
		int memSize;
		GetMemberValue(&optionProp[idx], object, curBuf, &memSize);
		curBuf += memSize;
		idx++;
	}

	*outMem = outBuf;
	*outSize = blockSize + sizeof(DWORD);
	return true;
}

inline static void DeSerilize(void *object, void *data)
{
	WindowDataContainer *windowData = (WindowDataContainer*)object;
	int idx, memCount = sizeof(optionProp) / sizeof(optionProp[0]);
	int blockSize = 0;

	unsigned char *curBuf = (unsigned char *)data;

	//memcpy(&blockSize, curBuf, sizeof(DWORD));
	//curBuf += sizeof(DWORD);

	idx = 0;
	while (idx < memCount)
	{
		int memSize;
		SetMemberValue(&optionProp[idx], object, curBuf, &memSize);
		curBuf += memSize;
		idx++;
	}
}


YZWinDisplayOutput::YZWinDisplayOutput()
{
	m_dxOutput = NULL;
	m_AdaptorContext = NULL;
	m_isSelectTag = FALSE;
	//m_isEditMode = FALSE;
	m_isEditMode = TRUE;
	m_criticalSection = (void*)new WDOLock();
}

YZWinDisplayOutput::YZWinDisplayOutput(void *dxOutput, void *adaptorContext)
{
	m_dxOutput = dxOutput;
	m_AdaptorContext = adaptorContext;
	m_isSelectTag = FALSE;
	//m_isEditMode = FALSE;
	m_isEditMode = TRUE;
	m_criticalSection = (void*)new WDOLock();
}

YZWinDisplayOutput::~YZWinDisplayOutput()
{
	if (NULL != m_criticalSection){
		delete (WDOLock*)m_criticalSection;
	}
}

int YZWinDisplayOutput::EdgeIndex(int idx)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	assert(idx >= 0);
	assert(idx < CTL_MAX);
	int tmpIdx = idx - CTL_T1;
	if (tmpIdx < CTL_HORIZONTAL_NUM)
		return CTL_MID_TOP;
	tmpIdx -= CTL_HORIZONTAL_NUM;
	if (tmpIdx < CTL_HORIZONTAL_NUM)
		return CTL_MID_BOTTOM;
	tmpIdx -= CTL_HORIZONTAL_NUM;
	if (tmpIdx < CTL_VERTICAL_NUM)
		return CTL_MID_LEFT;
	return CTL_MID_RIGHT;
}

int YZWinDisplayOutput::GetCtlPtCount(int idx)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	switch (idx)
	{
	case CTL_MID_TOP:
	case CTL_MID_BOTTOM:
		return CTL_HORIZONTAL_NUM;
	}
	return CTL_VERTICAL_NUM;
}

HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
#ifdef USE_D3D9_MATH
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#else
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
#ifdef USE_D3D9_MATH
	//dwShaderFlags |= D3D10_SHADER_DEBUG;
#else
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

#ifdef USE_D3D9_MATH
	dwShaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#else
	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
#endif

	ID3DBlob* pErrorBlob = nullptr;
#ifdef USE_D3D9_MATH
	//hr = D3D10CompileShader(NULL, 0, szFileName,
	//	NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, ppBlobOut, &pErrorBlob);

	//hr = D3DX10CompileFromFileA("render.fx", NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, NULL, NULL, ppBlobOut, &pErrorBlob, NULL);
	hr = D3DCompile(glsl, strlen(glsl), NULL, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
#else
	//wchar temp[128];
	//MultiByteToWideChar(CP_ACP, 0, szFileName, -1, temp, 100);
	//hr = D3DCompileFromFile(L"render.fx", nullptr, nullptr, szEntryPoint, szShaderModel,
	//	dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	hr = D3DCompile(glsl, strlen(glsl), NULL, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
#endif
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			"failed CompileShaderFromFile.", "Error", MB_OK);
		int ret = GetLastError();
		if (pErrorBlob)
		{
			
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

void YZWinDisplayOutput::CreateTargetView()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;
	// get the dxgi device
	IDXGIDevice* DXGIDevice = NULL;
	HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGIDevice); // COM stuff, hopefully you are familiar

	// create a swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// fill it in
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Width = window->width;
	swapChainDesc.BufferDesc.Height = window->height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = window->hWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	hr = adaptorContext->pIDXGIFactory->CreateSwapChain(DXGIDevice, &swapChainDesc, &window->swapChain);
	if (FAILED(hr))
	{
		int err = GetLastError();
		SHOW_ERROR();
	}
	DXGIDevice->Release();
	DXGIDevice = NULL;
	
	// get the backbuffer
	ID3D10Texture2D* backBuffer = NULL;
	hr = window->swapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&backBuffer);
	if (FAILED(hr))
	{
		int err = GetLastError();
		SHOW_ERROR();
	}

	// get the backbuffer desc
	D3D10_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);

	// create the render target view
	D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;

	// fill it in
	RTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	RTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	hr = device->CreateRenderTargetView(backBuffer, NULL, &window->renderTargetView);
	if (FAILED(hr))
	{
		int err = GetLastError();
		SHOW_ERROR();
	}
	backBuffer->Release();
	backBuffer = NULL;

	// Create depth stencil texture
	ID3D10Texture2D* depthStencil = NULL;
	D3D10_TEXTURE2D_DESC descDepth;

	// fill it in

	ZeroMemory(&descDepth, sizeof(D3D10_TEXTURE2D_DESC));
	descDepth.Width = window->width;
	descDepth.Height = window->height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_R32G32B32A32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
	descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL; // D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	hr = device->CreateTexture2D(&descDepth, NULL, &depthStencil);
	if (FAILED(hr))
	{
		int err = GetLastError();
		SHOW_ERROR();
	}
	// Create the depth stencil view
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	// fill it in
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthStencil, NULL, &window->depthStencilView);
	if (FAILED(hr))
	{
		int err = GetLastError();
		SHOW_ERROR();
	}

	InitializeVertex();
	SetVertexBuffer();
	SetDispalyMatrix();
	CreateRay(100, 200);
	CreateTagContext();
	DO_UpdateTilePos(0, 0, window->wSubTileNum, window->hSubTileNum);

	// Create the vertex input layout.
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile("render.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		int ret = GetLastError();
		MessageBox(nullptr,
			"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return;
	}

	ID3D10VertexShader *pVertexShader = NULL;
	// Create the vertex shader
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pVertexShader);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		pVSBlob->Release();
		return;
	}
	window->pVertexShader = pVertexShader;

	UINT numElements = ARRAYSIZE(vertexDesc);

	// Create the input layout
	ID3D10InputLayout *ppInputLayout;
	hr = device->CreateInputLayout(vertexDesc, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &ppInputLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		SHOW_ERROR();
	}

	// Set the input layout
	device->IASetInputLayout(ppInputLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile("render.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return;
	}

	// Create the pixel shader
	ID3D10PixelShader *g_pPixelShader = NULL;
	hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;
	window->pPixelShader = g_pPixelShader;


	pPSBlob = nullptr;
	hr = CompileShaderFromFile("render.fx", "PS1", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return;
	}

	g_pPixelShader = NULL;
	hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
	{
		SHOW_ERROR();
	}

	window->pPixelShader2 = g_pPixelShader;
}

void YZWinDisplayOutput::SetCtlPt(float ratio, int ctlIdx)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	if (IN_CTLPT_RANGE(ctlIdx))
	{
		int startPt = CTL_START_PT(ctlIdx);
		int endPt = CTL_END_PT(ctlIdx);
		XMVECTOR start = XMLoadFloat3(&window->controlVertex[startPt]);
		XMVECTOR end = XMLoadFloat3(&window->controlVertex[endPt]);
		XMVECTOR res = start + (end - start) * ratio;
		XMStoreFloat3(&window->controlVertex[ctlIdx], res);
		window->controlRatio[ctlIdx] = ratio;
	}
}

void YZWinDisplayOutput::ReSetCtlPt()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	for (int idx = CTL_T1; idx < CTL_MAX; idx++)
	{
		SetCtlPt(window->controlRatio[idx], idx);
		//SetCtlPt(0.66, idx + 1);
	}

	ReSetVertexWithCtlPt();
}

void YZWinDisplayOutput::ReSetVertexWithCtlPt()
{
	const int max_sub_pts = 500;
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	assert(window->wSubTileNum * (CTL_HORIZONTAL_NUM + 1) < max_sub_pts);
	assert(window->hSubTileNum * (CTL_VERTICAL_NUM + 1) < max_sub_pts);
	XMVECTOR subCtlPts[CTL_MID_RIGHT + 1][max_sub_pts]; 

	int currentIdx = 0;
	for (int idx = CTL_MID_TOP; idx <= CTL_MID_RIGHT; idx++)
	{
		int lastOffset = 0;
		int pos = 0;
		for (int ctlIdx = 0; ctlIdx <= GetCtlPtCount(idx); ctlIdx++)
		{
			XMFLOAT3 startPt, endPt;
			int startPtIdx, endPtIdx;
			if (ctlIdx == 0){
				startPtIdx = CTL_START_PT(currentIdx + CTL_T1);
				endPtIdx = CTL_T1 + currentIdx;
			}
			else if (ctlIdx == GetCtlPtCount(idx)){
				startPtIdx = CTL_T1 + currentIdx - 1;
				endPtIdx = CTL_END_PT(currentIdx - 1 + CTL_T1);
			}
			else{
				startPtIdx = CTL_T1 + currentIdx - 1;
				endPtIdx = CTL_T1 + currentIdx;
			}
			startPt = window->controlVertex[startPtIdx];
			endPt = window->controlVertex[endPtIdx];
			
 			int tileIdx = 0;
			
			int tileNum = (idx == CTL_MID_TOP || idx == CTL_MID_BOTTOM)
				? window->wSubTileNum : window->hSubTileNum;

			XMVECTOR start = XMLoadFloat3(&startPt);
			XMVECTOR end = XMLoadFloat3(&endPt);

			XMVECTOR delta = (end - start) / (float)tileNum;
			for (tileIdx = lastOffset;
				tileIdx < tileNum; 
				tileIdx += (GetCtlPtCount(idx) + 1), pos++)
			{
				XMFLOAT3 pt;
				XMStoreFloat3(&pt, start + delta * (float)tileIdx);
				subCtlPts[idx][pos] = start + delta * (float)tileIdx;
			}

			if (pos == tileNum)
			{
				//XMStoreFloat3(&pt, start + delta * tileIdx);
				subCtlPts[idx][pos] = start + delta * (float)tileIdx;
			}
			lastOffset = tileIdx - tileNum;

			if (ctlIdx < GetCtlPtCount(idx))
				currentIdx++;
		}
	}

	for (int yIdx = 0; yIdx <= window->hSubTileNum; yIdx++)
	{
		XMVECTOR startYPt = subCtlPts[CTL_MID_LEFT][yIdx];
		XMVECTOR endYPt = subCtlPts[CTL_MID_RIGHT][yIdx];
		for (int xIdx = 0; xIdx <= window->wSubTileNum; xIdx++)
		{
			XMVECTOR startXPt = subCtlPts[CTL_MID_BOTTOM][xIdx];
			XMVECTOR endXPt = subCtlPts[CTL_MID_TOP][xIdx];
			XMVECTOR result = XMVector2IntersectLine(startYPt, endYPt,
				startXPt, endXPt);
			XMFLOAT3 pos;
			XMStoreFloat3(&pos, result);
			XMStoreFloat3(&window->vertexMem[yIdx * (window->wSubTileNum + 1) + xIdx].Pos, result);
		}
	}

}

void YZWinDisplayOutput::SetTriangleIndices(void *buf, int xTileIdx, int yTileIdx)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	int idx = yTileIdx * window->wSubTileNum + xTileIdx;
	DWORD *indicesBuf = (DWORD *)buf + idx * 2 * 3;
	
	int offset = (window->wSubTileNum + 1) * yTileIdx + xTileIdx;
	indicesBuf[0] = offset + (window->wSubTileNum + 1);
	indicesBuf[1] = offset + 1;
	indicesBuf[2] = offset;

	indicesBuf += 3;

	indicesBuf[0] = offset + 1;
	indicesBuf[1] = offset + (window->wSubTileNum + 1);
	indicesBuf[2] = offset + (window->wSubTileNum + 1) + 1;
}

void YZWinDisplayOutput::InitializeVertex()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	int tile_size = 100;
	int x_tile_num = window->width / tile_size;
	int y_tile_num = window->height / tile_size;
	window->wSubTileNum = x_tile_num;
	window->hSubTileNum = y_tile_num;
	window->controlCount = 4 * (CTL_MAX + 1);
	window->controlVertex = new  XMFLOAT3[4 * (CTL_MAX + 1)];
	window->controlRatio = new float[4 * (CTL_MAX + 1)];

	int x_offset = (window->width % tile_size) / 2;
	int y_offset = (window->height % tile_size) / 2;
	int x_end = window->width - x_offset;
	int y_end = window->height - y_offset;

	float x_fraction = 2.0f / window->width;
	float y_fraction = x_fraction;// 2.0 / window->height;

	int vertex_num = (x_tile_num + 1) * (y_tile_num + 1);
	SimpleVertex *vertices = new SimpleVertex[vertex_num];

	int indices_num = x_tile_num * (y_tile_num + 1) * 2;//2 * (x_tile_num + 1) * 2 * (y_tile_num + 1);
	indices_num += y_tile_num * (x_tile_num + 1) * 2;

	DWORD *indices = new DWORD[indices_num];

	int planeIndicesCount = y_tile_num * x_tile_num * 2 * 3;
	DWORD *planeIndices = new DWORD[planeIndicesCount];

	float ratio = window->width / (float)window->height;

	int offsetIndex = 0;
	for (int y = 0; y <= y_tile_num; y++)
	{
		for (int x = 0; x < x_tile_num; x++)
		{
			indices[(y * x_tile_num + x) * 2] = y * (x_tile_num + 1) + x;
			indices[(y * x_tile_num + x) * 2 + 1] = y * (x_tile_num + 1) + x + 1;
			offsetIndex += 2;
		}
	}


	for (int x = 0; x <= x_tile_num; x++)
	{
		for (int y = 0; y < y_tile_num; y++)
		{
			indices[offsetIndex + (x * (y_tile_num)+y) * 2] = y * (x_tile_num + 1) + x;
			indices[offsetIndex + (x * (y_tile_num)+y) * 2 + 1] = (y + 1) * (x_tile_num + 1) + x;
		}
	}


	window->vertexMem = vertices;
	window->indicesMem = indices;
	window->planeIndicesMem = planeIndices;

	for (int y = 0; y < y_tile_num; y++)
	{
		for (int x = 0; x < x_tile_num; x++){
			SetTriangleIndices(planeIndices, x, y);
		}
	}


	window->controlVertex[CTL_LB] = XMFLOAT3(-1.0f + x_offset * x_fraction, -1.0f / ratio + y_offset * y_fraction, 0.5f);
	window->controlVertex[CTL_LT] = XMFLOAT3(-1.0f + x_offset * x_fraction, -1.0f / ratio + y_end * y_fraction, 0.5f);
	window->controlVertex[CTL_RB] = XMFLOAT3(-1.0f + x_end * x_fraction, -1.0f / ratio + y_offset * y_fraction, 0.5f);
	window->controlVertex[CTL_RT] = XMFLOAT3(-1.0f + x_end * x_fraction, -1.0f / ratio + y_end * y_fraction, 0.5f);

	int currentIdx = 0;
	for (int idx = CTL_MID_TOP; idx <= CTL_MID_RIGHT; idx++)
	{
		for (int subIdx = 0; subIdx < GetCtlPtCount(idx); subIdx++, currentIdx++)
			window->controlRatio[CTL_T1 + currentIdx] = 1.0f / (GetCtlPtCount(idx) + 1) * (subIdx + 1);
	}

	ReSetCtlPt();

	window->vertexCount = vertex_num;
	window->indicesCount = indices_num;
	window->planeIndicesCount = planeIndicesCount;
}

void YZWinDisplayOutput::SetVertexBuffer()
{
	HRESULT hr;
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;



	UpdateTexCoord();
	CreateVertexContext(window->vertexMem, sizeof(SimpleVertex)* window->vertexCount,
		window->indicesMem, sizeof(DWORD)* window->indicesCount,
		(void**)&window->vertexBuffer,
		(void**)&window->indicesBuffer);



	D3D10_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.MiscFlags = 0;
	//indexBufferDesc.StructureByteStride = 0;
	D3D10_SUBRESOURCE_DATA indexData;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = window->indicesMem;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;


	// Create the constant buffer
	indexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	indexBufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&indexBufferDesc, nullptr, &window->constantBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}
	
	indexBufferDesc.ByteWidth = sizeof(XMMATRIX);
	hr = device->CreateBuffer(&indexBufferDesc, nullptr, &window->colorBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}

	indexBufferDesc.ByteWidth = sizeof(OverLappedEdgeBuffer);
	hr = device->CreateBuffer(&indexBufferDesc, nullptr, &window->overLappedEdgeBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}
	
	indexData.pSysMem = window->planeIndicesMem;
	indexBufferDesc.ByteWidth = sizeof(DWORD)* window->planeIndicesCount;
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &window->planeIndexBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}

	
}

void YZWinDisplayOutput::UpdateVertexBuffer()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	ReSetCtlPt();

	//D3D10_MAPPED_SUBRESOURCE resource;
	void *mapBuf = NULL;
	window->vertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapBuf);
	memcpy(mapBuf, window->vertexMem, sizeof(SimpleVertex) * window->vertexCount);
	window->vertexBuffer->Unmap();
}

void YZWinDisplayOutput::SetDispalyMatrix()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	// Initialize the world matrix
	//window->g_World = XMMatrixIdentity();
	MJSMatrixIdentity(window->g_World);
	//window->g_World = XMMatrixTranslation(0, 0, -4.8);

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, window->viewZPos, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	// Initialize the world matrix
	MJSMatrixLookAtLH(window->g_View, Eye, At, Up);

	// Initialize the projection matrix
	MJSMatrixPerspectiveFovLH(window->g_Projection, XM_PIDIV2, window->width / (FLOAT)window->height, 0.5f, 100.0f);
}

void YZWinDisplayOutput::UpdateViewMatrix(float z)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	window->viewZPos += z;
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, window->viewZPos, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, window->viewZPos + 0.5f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	// Initialize the world matrix
	MJSMatrixLookAtLH(window->g_View, Eye, At, Up);
}

void YZWinDisplayOutput::CreateTagContext()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;
	XMFLOAT3 vt[4];
	DWORD indices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };
	CreateVertexContext(vt, sizeof(vt), indices, sizeof(indices),
		(void**)&window->tagVertexBuffer, (void**)&window->tagIndicesBuffer);
}

void YZWinDisplayOutput::CreateRay(int scrX, int scrY)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	XMFLOAT4 pos = XMFLOAT4((scrX - window->width / 2) / (float)window->width * 2, -(scrY - window->height / 2) / (float)window->height * 2, 0, 1);
	//XMFLOAT4 pos = XMFLOAT4(0, 0, 0, 1);
	window->RayDir = XMLoadFloat4(&pos);
	XMMATRIX tmpMatrix = window->g_Projection;
	XMMATRIX temp;// = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
    //window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	XMStoreFloat4(&pos, window->RayDir);
	tmpMatrix = window->g_View;
	//temp = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
	//window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	tmpMatrix = window->g_World;
	//temp = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
	//window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	XMStoreFloat4(&pos, window->RayDir);

	window->RayPos[0] = XMFLOAT3(pos.x / pos.w, pos.y / pos.w, pos.z / pos.w);
	window->RayPos[1] = XMFLOAT3(pos.x / pos.w * 5, pos.y / pos.w * 5, (pos.z / pos.w - window->viewZPos) * 5);

	DWORD indices[2] = { 0, 1 };
	CreateVertexContext(window->RayPos, sizeof(XMFLOAT3) * 2,
		indices, sizeof(DWORD) * 2, (void**)&window->rayBuffer, (void**)&window->rayIndicesBuffer);
}

void YZWinDisplayOutput::CreateVertexContext(void *pVertexData, int vertexSize, void *pIndicesData, int indicesSize, void **vertexBuf, void **indicesBuf)
{
	HRESULT hr;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;
	D3D10_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = vertexSize;
	vertexBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	//vertexBufferDesc.StructureByteStride = 0;

	D3D10_SUBRESOURCE_DATA vertexData;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = pVertexData;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	ID3D10Buffer *vertexBuffer = NULL;
	// Now create the vertex buffer.
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}
	*vertexBuf = vertexBuffer;

	D3D10_BUFFER_DESC indexBufferDesc;
	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D10_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = indicesSize;
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	//indexBufferDesc.StructureByteStride = 0;

	D3D10_SUBRESOURCE_DATA indexData;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = pIndicesData;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D10Buffer *indexBuffer = NULL;
	// Create the index buffer.
	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	if (FAILED(hr))
	{
		SHOW_ERROR();
		return;
	}
	*indicesBuf = indexBuffer;
}

void YZWinDisplayOutput::UpdateRay(int scrX, int scrY)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;
	XMFLOAT4 pos = XMFLOAT4((scrX - window->width / 2) / (float)window->width * 2, -(scrY - window->height / 2) / (float)window->height * 2, 0, 1);
	//XMFLOAT4 pos = XMFLOAT4(0, 0, 0, 1);
	window->RayDir = XMLoadFloat4(&pos);
	XMMATRIX tmpMatrix = window->g_Projection;
	XMMATRIX temp;// = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
	//window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	XMStoreFloat4(&pos, window->RayDir);
	tmpMatrix = window->g_View;
	//temp = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
	//window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	tmpMatrix = window->g_World;
	//temp = XMMatrixInverse(NULL, tmpMatrix);
	MJSMatrixInverse(temp, tmpMatrix);
	//window->RayDir = XMVector4Transform(window->RayDir, temp);
	MJSVector4Transform(window->RayDir, temp);
	XMStoreFloat4(&pos, window->RayDir);
	float aaa = pos.x / pos.w;
	//window->RayPos[0] = XMFLOAT3(0, 0, pos.z / pos.w);
	window->RayPos[0] = XMFLOAT3(pos.x / pos.w, pos.y / pos.w, pos.z / pos.w);
	window->RayPos[1] = XMFLOAT3(pos.x / pos.w + pos.x / pos.w * (-2 * window->viewZPos),
		pos.y / pos.w + pos.y / pos.w * (-2 * window->viewZPos),
		pos.z / pos.w + (pos.z / pos.w - window->viewZPos) * (-2 * window->viewZPos));

	//D3D10_MAPPED_SUBRESOURCE resource;
	void *mapBuf = NULL;
	window->rayBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapBuf);
	memcpy(mapBuf, window->RayPos, sizeof(window->RayPos));
	window->rayBuffer->Unmap();
	if (m_isSelectTag ){
	//device->UpdateSubresource(window->rayBuffer, 0, NULL, window->RayPos, 0, 0);
		XMFLOAT3 vt = window->controlVertex[m_selectTagIdx];
		if (IN_CTLPT_RANGE(m_selectTagIdx))
		{
			int startIdx = CTL_START_PT(m_selectTagIdx);
			int endIdx = CTL_END_PT(m_selectTagIdx);
			if (CHECK_HORIZONTAL(m_selectTagIdx)){
				window->controlRatio[m_selectTagIdx] = (window->RayPos[1].x - window->controlVertex[startIdx].x) /
					(window->controlVertex[endIdx].x - window->controlVertex[startIdx].x);
			}
			else{
				window->controlRatio[m_selectTagIdx] = (window->RayPos[1].y - window->controlVertex[startIdx].y) /
					(window->controlVertex[endIdx].y - window->controlVertex[startIdx].y);
			}
		}
		else
		{
			vt.x = vt.x + (window->RayPos[1].x - window->lastClickPos.x);
			vt.y = vt.y + (window->RayPos[1].y - window->lastClickPos.y);
			window->controlVertex[m_selectTagIdx] = XMFLOAT3(vt.x, vt.y, vt.z);
		}
		window->lastClickPos = window->RayPos[1];
		UpdateVertexBuffer();

		if (!CheckInPtRect(&window->controlVertex[m_selectTagIdx])){
			m_isSelectTag = FALSE;
		}
	}		
}

void YZWinDisplayOutput::UpdateTexCoord()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	int currentIdx = 0;

	int startPtIdx, endPtIdx;
	startPtIdx = IDX_START_PT(CTL_MID_BOTTOM);
	endPtIdx = IDX_END_PT(CTL_MID_BOTTOM);

	XMVECTOR startLeftPt = XMLoadFloat2(&window->texCoord[startPtIdx]);
	XMVECTOR startRightPt = XMLoadFloat2(&window->texCoord[endPtIdx]);

	startPtIdx = IDX_START_PT(CTL_MID_TOP);
	endPtIdx = IDX_END_PT(CTL_MID_TOP);

	XMVECTOR deltaLeftY = (XMLoadFloat2(&window->texCoord[startPtIdx]) - startLeftPt) / (float)window->hSubTileNum;
	XMVECTOR deltaRightY = (XMLoadFloat2(&window->texCoord[endPtIdx]) - startRightPt) / (float)window->hSubTileNum;

	for (int yIdx = 0; yIdx <= window->hSubTileNum; yIdx++)
	{
		XMVECTOR startPt = startLeftPt + (float)yIdx * deltaLeftY;
		XMVECTOR endPt = startRightPt + (float)yIdx * deltaRightY;
		XMVECTOR delta = (endPt - startPt) / (float)window->wSubTileNum;
		for (int xIdx = 0; xIdx <= window->wSubTileNum; xIdx++)
		{
			XMFLOAT2 pt;
			XMVECTOR currentPt = startPt + delta * (float)xIdx;
			XMStoreFloat2(&pt, currentPt);
			window->vertexMem[yIdx * (window->wSubTileNum + 1) + xIdx].Tex = pt;
		}
	}
}

void YZWinDisplayOutput::ClickUp()
{
	if (!m_isEditMode) return;
	m_isSelectTag = FALSE;
}

void YZWinDisplayOutput::ClickDown()
{
	if (!m_isEditMode) return;
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	for (int idx = 0; idx < CTL_MAX; idx++){
		if (CheckInPtRect(&window->controlVertex[idx])){
			m_isSelectTag = TRUE;
			m_selectTagIdx = idx;

			window->lastClickPos = window->RayPos[1];
			return;
		}
	}
}

void YZWinDisplayOutput::SwitchEditMode()
{
	m_isEditMode = !m_isEditMode;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	void *wdo = (void*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (!wdo)
	{
		if (msg != WM_NCCREATE)
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		CREATESTRUCTW *cs = (CREATESTRUCTW*)lParam;
		wdo = cs->lpCreateParams;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)wdo);
	}
	YZWinDisplayOutput *displayOutput = reinterpret_cast<YZWinDisplayOutput*>(wdo);
	switch (msg)
	{
	case WM_PAINT:
		displayOutput->DO_Render();
		displayOutput->DO_Present();
		break;
	case WM_DESTROY: //销毁  
		PostQuitMessage(0);//终止请求  
		break;
	case WM_KEYDOWN:  //键按下  
		if (wParam == VK_ESCAPE)//Esc键  
			DestroyWindow(hwnd);
		if (wParam == VK_UP)
			((YZWinDisplayOutput*)displayOutput)->UpdateViewMatrix(-0.1f);
		if (wParam == VK_DOWN)
			((YZWinDisplayOutput*)displayOutput)->UpdateViewMatrix(0.1f);
		if (wParam == VK_CONTROL)
			((YZWinDisplayOutput*)displayOutput)->SwitchEditMode();
		displayOutput->DO_Render();
		displayOutput->DO_Present();
		break;
	case WM_LBUTTONDOWN:
		((YZWinDisplayOutput*)displayOutput)->ClickDown();
		break;
	case WM_LBUTTONUP:
		((YZWinDisplayOutput*)displayOutput)->ClickUp();
		break;
	case WM_MOUSEMOVE:
		((YZWinDisplayOutput*)displayOutput)->UpdateRay(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		displayOutput->DO_Render();
		displayOutput->DO_Present();
		break;
	}
	//调用缺省的窗口过程来为应用程序没有处理的任何窗口消息提供缺省的处理。  
	//该函数确保每一个消息得到处理  
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int YZWinDisplayOutput::Initialize(int index)
{
	DXGI_OUTPUT_DESC outputDesc;
	IDXGIOutput* p_Output = reinterpret_cast<IDXGIOutput*>(m_dxOutput);
	p_Output->GetDesc(&outputDesc);
	int x = outputDesc.DesktopCoordinates.left;
	int y = outputDesc.DesktopCoordinates.top;
	int width = outputDesc.DesktopCoordinates.right - x;
	int height = outputDesc.DesktopCoordinates.bottom - y;

	// Don't forget to clean this up. And all D3D COM objects.
	WindowDataContainer* window = new WindowDataContainer;
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D9App";

	//窗口注册  
	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}
	window->hWnd = CreateWindow("Direct3D9App",
		"Direct3D9App",
		WS_POPUP,
		x,
		y,
		width,
		height,
		NULL,
		0,
		(NULL),
		this);
	int err = GetLastError();
	// show the window
	ShowWindow(window->hWnd, SW_SHOWDEFAULT);

	// set width and height
	window->width = width;
	window->height = height;

	// shove it in the std::vector
	//windowsArray.push_back(window);

	//if first window, associate it with DXGI so it can jump in
	// when there is something of interest in the message queue
	// think fullscreen mode switches etc. MSDN for more info.
	//if (i == 0)
	//	factory->MakeWindowAssociation(window->hWnd, 0);
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	IDXGIFactory * pIDXGIFactory = adaptorContext->pIDXGIFactory;
	if (index == 0)
		pIDXGIFactory->MakeWindowAssociation(window->hWnd, 0);

	m_WindowContext = window;

	CreateTargetView();
	
	return 0;
}

void YZWinDisplayOutput::SetTilesOverlapped(int top, int bottom, int left, int right)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	if (window->leftOverlapped != left){
		window->leftOverlapped = left;
	}
	if (window->rightOverlapped != right){
		window->rightOverlapped = right;
	}
	if (window->topOverlapped != top){
		window->topOverlapped = top;
	}
	if (window->bottomOverlapped != bottom){
		window->bottomOverlapped = bottom;
	}

	
	window->overLappedEdge.overLappedEdge.x = window->texCoord[CTL_LB].x +
		(window->texCoord[CTL_RB].x - window->texCoord[CTL_LB].x) * window->leftOverlapped / window->wSubTileNum;
	window->overLappedEdge.overLappedEdge.y = window->texCoord[CTL_LB].x +
		(window->texCoord[CTL_RB].x - window->texCoord[CTL_LB].x) * (window->wSubTileNum - window->rightOverlapped) / window->wSubTileNum;

	window->overLappedEdge.overLappedEdge.z = window->texCoord[CTL_RB].y +
		(window->texCoord[CTL_RT].y - window->texCoord[CTL_RB].y) * window->bottomOverlapped / window->hSubTileNum;
	window->overLappedEdge.overLappedEdge.w = window->texCoord[CTL_RB].y +
		(window->texCoord[CTL_RT].y - window->texCoord[CTL_RB].y) * (window->hSubTileNum - window->topOverlapped) / window->hSubTileNum;

	window->overLappedEdge.textCoord = XMFLOAT4(window->texCoord[CTL_LB].x, window->texCoord[CTL_RT].x,
		window->texCoord[CTL_LB].y, window->texCoord[CTL_RT].y);

}

void YZWinDisplayOutput::DO_UpdateTilePos(int xOffset, int yOffset, int tileTotalWNum, int tileTotalHNum)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	window->m_xTileOffset = xOffset;
	window->m_yTileOffset = yOffset;
	window->m_tileTotalWNum = tileTotalWNum;
	window->m_tileTotalHNum = tileTotalHNum;

	float texStartX = 1.0f * xOffset / tileTotalWNum;
	float texEndX = 1.0f * (xOffset + window->wSubTileNum) / tileTotalWNum;
	float texStartY = 1.0f * yOffset / tileTotalHNum;
	float texEndY = 1.0f * (yOffset + window->hSubTileNum) / tileTotalHNum;

	window->texCoord[CTL_LB] = XMFLOAT2(texStartX, texEndY);
	window->texCoord[CTL_RB] = XMFLOAT2(texEndX, texEndY);
	window->texCoord[CTL_RT] = XMFLOAT2(texEndX, texStartY);
	window->texCoord[CTL_LT] = XMFLOAT2(texStartX, texStartY);

	UpdateTexCoord();

	void *mapBuf = NULL;
	window->vertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapBuf);
	memcpy(mapBuf, window->vertexMem, sizeof(SimpleVertex)* window->vertexCount);
	window->vertexBuffer->Unmap();
}

bool YZWinDisplayOutput::CheckInPtRect(void *vetex)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	XMFLOAT3 *vt = (XMFLOAT3*)vetex;
	float cap = 0.02f;
	XMFLOAT3 vtRect[] = {
		XMFLOAT3(vt->x - cap, vt->y - cap, vt->z),
		XMFLOAT3(vt->x - cap, vt->y + cap, vt->z),
		XMFLOAT3(vt->x + cap, vt->y + cap, vt->z),
		XMFLOAT3(vt->x + cap, vt->y - cap, vt->z),
	};

	if (vtRect[0].x < window->RayPos[1].x
		&&vtRect[2].x > window->RayPos[1].x
		&&vtRect[0].y < window->RayPos[1].y
		&&vtRect[2].y > window->RayPos[1].y){
		return TRUE;
	}
	return FALSE;
}

void YZWinDisplayOutput::DO_SetColor(void *color)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	memcpy(&window->colormatrix, color, sizeof(window->colormatrix));
}	

void YZWinDisplayOutput::DrawTag(void *vetex)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;


	XMFLOAT3 *vt = (XMFLOAT3*)vetex;
	float cap = 0.02f;
	XMFLOAT3 vtRect[] = {
		XMFLOAT3(vt->x - cap, vt->y - cap, vt->z),
		XMFLOAT3(vt->x - cap, vt->y + cap, vt->z),
		XMFLOAT3(vt->x + cap, vt->y + cap, vt->z),
		XMFLOAT3(vt->x + cap, vt->y - cap, vt->z),
	};

	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	void *mapBuf = NULL;
	window->tagVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mapBuf);
	memcpy(mapBuf, vtRect, sizeof(vtRect));
	window->tagVertexBuffer->Unmap();

	//ConstantBuffer cb;
	//cb.mPaintColor = XMFLOAT4(0.0, 1.0, 0.0, 1.0);
	//device->UpdateSubresource(window->constantBuffer, 0, nullptr, &cb, 0, 0);

	device->IASetVertexBuffers(0, 1, &window->tagVertexBuffer, &stride, &offset);
	device->IASetIndexBuffer(window->tagIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);
	device->VSSetShader(window->pVertexShader);
	device->VSSetConstantBuffers(0, 1, &window->constantBuffer);
	//device->PSSetConstantBuffers(0, 1, &window->constantBuffer);
	device->PSSetShader(window->pPixelShader);
	device->DrawIndexed(8, 0, 0);
}

void YZWinDisplayOutput:: DO_Render()
{
	CSync sync(m_criticalSection);
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	// There is the answer to your second question:
	device->OMSetRenderTargets(1, &window->renderTargetView, window->depthStencilView);
	// Don't forget to adjust the viewport, in fullscreen it's not important...
	D3D10_VIEWPORT Viewport;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = window->width;
	Viewport.Height = window->height;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	device->RSSetViewports(1, &Viewport);



	const FLOAT ColorRGBA[4] = { 0, 0, 0, 1 };
	device->ClearRenderTargetView(window->renderTargetView, ColorRGBA);
	//device->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	device->ClearDepthStencilView(window->depthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	device->OMSetDepthStencilState(0, 0);
	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	device->OMSetBlendState(0, blendFactors, 0xffffffff);

	ConstantBuffer cb;
	XMMATRIX ex = window->g_World;
	XMMATRIX tmp;
	MJSMatrixTranspose(tmp, ex);

	cb.mWorld = tmp;
	ex = window->g_View;
	//cb.mView = XMMatrixTranspose(ex);
	MJSMatrixTranspose(cb.mView, ex);
	ex = window->g_Projection;
	//cb.mProjection = XMMatrixTranspose(ex);
	MJSMatrixTranspose(cb.mProjection, ex);
	
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	
	//device->IASetInputLayout(mVertexLayout);

	if (m_isEditMode)
	{
		cb.mPaintColor = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		device->UpdateSubresource(window->constantBuffer, 0, nullptr, &cb, 0, 0);
		device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		device->IASetVertexBuffers(0, 1, &window->vertexBuffer, &stride, &offset);
		device->IASetIndexBuffer(window->indicesBuffer, DXGI_FORMAT_R32_UINT, 0);
		device->VSSetShader(window->pVertexShader);
		device->VSSetConstantBuffers(0, 1, &window->constantBuffer);
		device->PSSetConstantBuffers(0, 1, &window->constantBuffer);
		device->PSSetShader(window->pPixelShader);
		device->DrawIndexed(window->indicesCount, 0, 0);


		cb.mPaintColor = XMFLOAT4(0.0, 1.0, 0.0, 1.0);
		device->UpdateSubresource(window->constantBuffer, 0, nullptr, &cb, 0, 0);
		device->PSSetShader(window->pPixelShader);
		for (int idx = 0; idx < CTL_MAX; idx++)
		{
			DrawTag(&window->controlVertex[idx]);
		}
	}
	else{
		device->UpdateSubresource(window->colorBuffer, 0, nullptr, &window->colormatrix, 0, 0);
		device->UpdateSubresource(window->overLappedEdgeBuffer, 0, nullptr, &window->overLappedEdge, 0, 0);
		device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		device->IASetVertexBuffers(0, 1, &window->vertexBuffer, &stride, &offset);
		device->IASetIndexBuffer(window->planeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		device->VSSetShader(window->pVertexShader);
		device->PSSetConstantBuffers(1, 1, &window->colorBuffer);
		device->PSSetConstantBuffers(2, 1, &window->overLappedEdgeBuffer);
		device->VSSetConstantBuffers(0, 1, &window->constantBuffer);
		device->PSSetShader(window->pPixelShader2);
		device->PSSetShaderResources(0, 3, adaptorContext->pShaderPlane);
		device->PSSetSamplers(0, 3, adaptorContext->pSamplerLinear);
		device->DrawIndexed(window->planeIndicesCount, 0, 0);
	}

}

void YZWinDisplayOutput::DO_DrawOnScreen(const TScreenPos *scrPos, int posNum)
{
	
}

void YZWinDisplayOutput::DO_Present()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	ID3D10Device *device = adaptorContext->d3dDevice;

	window->swapChain->Present(0, 0);
}

void YZWinDisplayOutput::DO_SetData(void *mem, int size)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	DeSerilize(window, mem);
	DO_RefreshLayout();
}

void YZWinDisplayOutput::DO_GetData(void **mem, int *size)
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	Serilize(window, mem, size);
}

void YZWinDisplayOutput::DO_RefreshLayout()
{
	WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
	UpdateTexCoord();
	UpdateVertexBuffer();
	//SetVertexBuffer();
	SetDispalyMatrix();
	//CreateRay(100, 200);
	CreateTagContext();
	DO_UpdateTilePos(window->m_xTileOffset, window->m_yTileOffset,
		window->m_tileTotalWNum, window->m_tileTotalHNum);
}