//#include "stdafx.h"

#include <stdio.h>
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3d10misc.h>
#include <vector>

#ifdef USE_D3D9_MATH
#include "video/out/multid3d/YZDirector3DAdapter.h"
#include "video/out/multid3d/YZWinDisplayOutput.h"
#include "video/out/multid3d/YUDirect3DCommon.h"
extern "C"
{
	HRESULT WINAPI D3DX10CreateDevice(IDXGIAdapter *pAdapter,
                                  D3D10_DRIVER_TYPE DriverType,
                                  HMODULE Software,
                                  UINT Flags,
                                  ID3D10Device **ppDevice);
}
#else
#include <D3DX10Core.h>
#include "YZDirector3DAdapter.h"
#include "YZWinDisplayOutput.h"
#include "YUDirect3DCommon.h"
#pragma comment(lib, "D3D10_1.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D10.lib")
#pragma comment (lib, "d3dx10.lib")
#endif


struct DisplayContainer
{
	vector<YZDisplayOutput*> vOutputObjects;
};

YZDirector3DAdapter::YZDirector3DAdapter()
{
	m_IDXGIAdapter = NULL;
	m_AdaptorContext = new YZDirect3DAdaptorData();
	m_displayContainer = new DisplayContainer;
}


YZDirector3DAdapter::~YZDirector3DAdapter()
{
	if (NULL != m_displayContainer){
		delete (DisplayContainer*)m_displayContainer;
	}

	if (NULL != m_AdaptorContext){
		delete (YZDirect3DAdaptorData*)m_AdaptorContext;
	}
}


int YZDirector3DAdapter::Initialize()
{
	ID3D10Device *d3dDevice1;
	IDXGIDevice * pDXGIDevice;

	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	HRESULT hr = D3DX10CreateDevice(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG, &d3dDevice1);
	
	if (FAILED(hr))
	{
		return -1;
	}
	adaptorContext->d3dDevice = d3dDevice1;

	hr = d3dDevice1->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
	if (FAILED(hr))
	{
		return -1;
	}

	IDXGIAdapter * pDXGIAdapter = NULL;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if (FAILED(hr))
	{
		return -1;
	}

	IDXGIFactory * pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
	adaptorContext->pIDXGIFactory = pIDXGIFactory;

	DA_FillOutputObjects();

	return 0;
}

int YZDirector3DAdapter::DA_FillOutputObjects()
{
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	if (NULL == adaptorContext)
	{
		return -1;
	}

	IDXGIAdapter* adapter = NULL;
	IDXGIFactory * pIDXGIFactory = adaptorContext->pIDXGIFactory;
	// remember, we assume there's only one adapter (example purposes)
	for (int i = 0; DXGI_ERROR_NOT_FOUND != pIDXGIFactory->EnumAdapters(i, &adapter); ++i)
	{

		// get the description of the adapter, assuming no failure
		DXGI_ADAPTER_DESC adapterDesc;
		HRESULT hr = adapter->GetDesc(&adapterDesc);

		// Getting the outputs active on our adapter
		//EnumOutputsOnAdapter();
		IDXGIOutput* output = NULL;
		for (int i = 0; DXGI_ERROR_NOT_FOUND != adapter->EnumOutputs(i, &output); ++i)
		{

			// get the description
			DXGI_OUTPUT_DESC outputDesc;
			HRESULT hr = output->GetDesc(&outputDesc);

			dpContainer->vOutputObjects.push_back(new YZWinDisplayOutput(output, m_AdaptorContext));
		}

	}

	for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
	{
		dpContainer->vOutputObjects[i]->Initialize(i);
	}

	for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
	{
		//dpContainer->vOutputObjects[i]->DO_UpdateTilePos(0, 0, 25, 16);
		dpContainer->vOutputObjects[i]->SetTilesOverlapped(0, 0, 0, 0);
	}

	CreateTexture(848, 480, 16);
	return 0;
}

void YZDirector3DAdapter::DO_Render()
{
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
	{
		dpContainer->vOutputObjects[i]->DO_Render();
	}
}

void YZDirector3DAdapter::DO_SetColor(void *color)
{
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
	{
		dpContainer->vOutputObjects[i]->DO_SetColor(color);
	}
}

void YZDirector3DAdapter::UpdateTexture(int width, int height, int bpp, void *texBuf, int plane)
{
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	adaptorContext->UpdateTexture(width, height, 16, texBuf, plane);
}


void YZDirector3DAdapter::CreateTexture( int width, int height, int bpp)
{
	YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
	WORD *buf = new WORD[width * height];
	for (int i = 0; i < width * height; i++){
		if ((i / width) % 2){
			buf[i] = 0xff00;
		}
		else{
			buf[i] = 0x0000;
		}
	}
	for (int n = 0; n < 3; n++)
		adaptorContext->CreateTexture(width, height, 16, buf, n);
	delete[]buf;
}

void YZDirector3DAdapter::D3A_Present()
{
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
	{
		dpContainer->vOutputObjects[i]->DO_Present();
	}
}

void YZDirector3DAdapter::DO_SaveConfig(const char *filename)
{
	FILE *fp;
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	errno_t err = fopen_s(&fp, filename, "wb");
	if (NULL != fp){
		for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
		{
			void *mem;
			int size;
			dpContainer->vOutputObjects[i]->DO_GetData(&mem, &size);
			int ret = fwrite(mem, 1, size, fp);
			if (ret != size){
				
			}
		}
		fclose(fp);
	}
}

void YZDirector3DAdapter::DO_LoadConfig(const char *filename)
{
	FILE *fp;
	DisplayContainer *dpContainer = (DisplayContainer*)m_displayContainer;
	errno_t err = fopen_s(&fp, filename, "rb");
	if (NULL != fp){
		for (int i = 0; i < dpContainer->vOutputObjects.size(); i++)
		{
			void *mem;
			int size;
			int ret = fread(&size, 1, 4, fp);
			mem = new unsigned char[size];
			ret = fread(mem, 1, size, fp);
			dpContainer->vOutputObjects[i]->DO_SetData(mem, size);
			
		}
		fclose(fp);
	}
}