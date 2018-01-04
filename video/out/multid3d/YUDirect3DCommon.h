#pragma once

#include <stdio.h>
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3d10misc.h>


#ifdef USE_D3D9_MATH
inline UINT D3D10CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT MipLevels) {
	return MipSlice + ArraySlice * MipLevels;
}
#endif

struct YZDirect3DAdaptorData
{
	ID3D10Device *d3dDevice;
	IDXGISwapChain *pSwapChain;
	IDXGIFactory * pIDXGIFactory;

#define MAX_PLANE	3
	ID3D10Texture2D *pPlaneTexure[MAX_PLANE];
	ID3D10ShaderResourceView *pShaderPlane[MAX_PLANE];
	ID3D10SamplerState *pSamplerLinear[MAX_PLANE];
	int texWidth[MAX_PLANE];
	int texHeight[MAX_PLANE];
	int bpp[MAX_PLANE];

	void CreateTexture(int width, int height, int bpp, void *buf, int plane)
	{
		HRESULT hr;
		//WindowDataContainer *window = (WindowDataContainer*)m_WindowContext;
		//YZDirect3DAdaptorData *adaptorContext = reinterpret_cast<YZDirect3DAdaptorData*>(m_AdaptorContext);
		ID3D10Device *device = d3dDevice;
		int format = DXGI_FORMAT_R16_UNORM; // DXGI_FORMAT_R8G8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;


		// Create the render target texture
		D3D10_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = (DXGI_FORMAT)format;
		desc.SampleDesc.Count = 1;
		//desc.Usage = D3D10_USAGE_DEFAULT;
		desc.Usage = D3D10_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;

		//window->planeMem = buf;
		texWidth[plane] = width;
		texHeight[plane] = height;
		this->bpp[plane] = bpp;
		//memset(buf, 0x12, width * height * 4);

		D3D10_SUBRESOURCE_DATA subResource;
		ZeroMemory(&subResource, sizeof(subResource));
		subResource.pSysMem = buf;
		subResource.SysMemPitch = (width * bpp + 7) / 8;
		subResource.SysMemSlicePitch = subResource.SysMemPitch * height;

		ID3D10Texture2D *pRenderTarget = NULL;
		hr = device->CreateTexture2D(&desc, &subResource, &pRenderTarget);
		if (FAILED(hr)){

			char tmpstr[200];
			sprintf_s(tmpstr, sizeof(tmpstr), "Error code : %lX", hr);
			OutputDebugString(tmpstr);
		}
		pPlaneTexure[plane] = pRenderTarget;

		ID3D10ShaderResourceView *pShaderResourceView;
		D3D10_SHADER_RESOURCE_VIEW_DESC shaderdesc;
		ZeroMemory(&shaderdesc, sizeof(shaderdesc));
		shaderdesc.Format = (DXGI_FORMAT)format;
		shaderdesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		shaderdesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(pRenderTarget, NULL, &pShaderResourceView);
		if (FAILED(hr)){

			char tmpstr[200];
			sprintf_s(tmpstr, sizeof(tmpstr), "Error code : %lX", hr);
			OutputDebugString(tmpstr);
		}
		pShaderPlane[plane] = pShaderResourceView;

		// Create the sample state
		D3D10_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D10_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D10_FLOAT32_MAX;
		hr = device->CreateSamplerState(&sampDesc, &pSamplerLinear[plane]);
		if (FAILED(hr)){

			char tmpstr[200];
			sprintf_s(tmpstr, sizeof(tmpstr), "Error code : %lX", hr);
			OutputDebugString(tmpstr);
		}

		//device->UpdateSubresource(pRenderTarget, 0, NULL, buf, width * 4, width * height * 4);
		//UpdateTexCoord();
	}

	void UpdateTexture(int width, int height, int bpp, void *texBuf, int plane)
	{
		ID3D10Device *device = d3dDevice;

		if (texWidth[plane] != width
			|| texHeight[plane] != height
			|| this->bpp[plane] != bpp){
			pPlaneTexure[plane]->Release();
			pPlaneTexure[plane] = NULL;
			pShaderPlane[plane]->Release();
			pShaderPlane[plane] = NULL;
			pSamplerLinear[plane]->Release();
			pSamplerLinear[plane] = NULL;
			CreateTexture(width, height, bpp, texBuf, plane);
		}

		D3D10_MAPPED_TEXTURE2D mapBuf;
		pPlaneTexure[plane]->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mapBuf);
		//mapBuf.RowPitch = (width * 16 + 7) / 8;
		//memcpy(mapBuf.pData, texBuf, mapBuf.RowPitch * height);
		for (int row = 0; row < height; row++){
			memcpy((unsigned char *)mapBuf.pData + mapBuf.RowPitch * row,
				(unsigned char *)texBuf + width * 2 * row, width * 2);
		}
		pPlaneTexure[plane]->Unmap(D3D10CalcSubresource(0, 0, 1));
	}
};