/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "surfaceDirect3D11.h"

#if defined(SDL_VIDEO_RENDER_D3D11)
#include <d3d11_1.h>

//#define _USE_D3D_SHADER_COMPILER_
#ifdef SDL_VIDEO_DRIVER_WINRT
//The WinRT part of code are copied from SDL direct3d11 renderer so I have no idea if they work

#include <DXGI.h>
#if NTDDI_VERSION > NTDDI_WIN8
#include <DXGI1_3.h>
#endif

#include <windows.ui.core.h>
#include <windows.graphics.display.h>

#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#include <windows.ui.xaml.media.dxinterop.h>
/* TODO, WinRT, XAML: get the ISwapChainBackgroundPanelNative from something other than a global var */
extern ISwapChainBackgroundPanelNative * WINRT_GlobalSwapChainBackgroundPanelNative;
#endif  /* WINAPI_FAMILY == WINAPI_FAMILY_APP */

using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

#endif  /* SDL_VIDEO_DRIVER_WINRT */

#include <SDL2/SDL_syswm.h>

#ifdef _USE_D3D_SHADER_COMPILER_
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#endif

#if defined(__cplusplus) && !defined(CINTERFACE)
#define ID3D11Device_QueryInterface(This,riid,ppvObject) (This)->QueryInterface(riid,ppvObject)
#define ID3D11Device_CreateBuffer(This,pDesc,pInitialData,ppBuffer) (This)->CreateBuffer(pDesc,pInitialData,ppBuffer)
#define ID3D11Device_CreateSamplerState(This,pSamplerDesc,ppSamplerState) (This)->CreateSamplerState(pSamplerDesc,ppSamplerState)
#define ID3D11Device_CreateRasterizerState(This,pRasterizerDesc,ppRasterizerState) (This)->CreateRasterizerState(pRasterizerDesc,ppRasterizerState)
#define ID3D11Device_CreateBlendState(This,pBlendStateDesc,ppBlendState) (This)->CreateBlendState(pBlendStateDesc,ppBlendState)
#define ID3D11Device_CreateVertexShader(This,pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader) (This)->CreateVertexShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader)
#define ID3D11Device_CreateInputLayout(This,pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout) (This)->CreateInputLayout(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout)
#define ID3D11Device_CreatePixelShader(This,pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader) (This)->CreatePixelShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader)
#define ID3D11Device_CreateRenderTargetView(This,pResource,pDesc,ppRTView) (This)->CreateRenderTargetView(pResource,pDesc,ppRTView)
#define ID3D11Device_CreateTexture2D(This,pDesc,pInitialData,ppTexture2D) (This)->CreateTexture2D(pDesc,pInitialData,ppTexture2D)
#define ID3D11Device_CreateShaderResourceView(This,pResource,pDesc,ppSRView) (This)->CreateShaderResourceView(pResource,pDesc,ppSRView)
#define ID3D11DeviceContext_QueryInterface(This,riid,ppvObject) (This)->QueryInterface(riid,ppvObject)
#define ID3D11DeviceContext_UpdateSubresource(This,pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch) (This)->UpdateSubresource(pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch)
#define ID3D11DeviceContext_Map(This,pResource,Subresource,MapType,MapFlags,pMappedResource) (This)->Map(pResource,Subresource,MapType,MapFlags,pMappedResource)
#define ID3D11DeviceContext_Unmap(This,pResource,Subresource) (This)->Unmap(pResource,Subresource)
#define ID3D11DeviceContext_CopySubresourceRegion(This,pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox) (This)->CopySubresourceRegion(pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox)
#define ID3D11DeviceContext_IASetVertexBuffers(This,StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets) (This)->IASetVertexBuffers(StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets)
#define ID3D11DeviceContext_IASetIndexBuffer(This,pIndexBuffer,Format,Offset) (This)->IASetIndexBuffer(pIndexBuffer,Format,Offset)
#define ID3D11DeviceContext_IASetPrimitiveTopology(This,Topology) (This)->IASetPrimitiveTopology(Topology)
#define ID3D11DeviceContext_IASetInputLayout(This,pInputLayout) (This)->IASetInputLayout(pInputLayout)
#define ID3D11DeviceContext_Draw(This,VertexCount,StartVertexLocation) (This)->Draw(VertexCount,StartVertexLocation)
#define ID3D11DeviceContext_DrawIndexed(This,IndexCount,StartIndexLocation,BaseVertexLocation) (This)->DrawIndexed(IndexCount,StartIndexLocation,BaseVertexLocation)
#define ID3D11DeviceContext_VSSetShader(This,pVertexShader,ppClassInstances,NumClassInstances) (This)->VSSetShader(pVertexShader,ppClassInstances,NumClassInstances)
#define ID3D11DeviceContext_VSSetConstantBuffers(This,StartSlot,NumBuffers,ppConstantBuffers) (This)->VSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers)
#define ID3D11DeviceContext_OMSetRenderTargets(This,NumViews,ppRenderTargetViews,pDepthStencilView) (This)->OMSetRenderTargets(NumViews,ppRenderTargetViews,pDepthStencilView)
#define ID3D11DeviceContext_OMSetBlendState(This,pBlendState,BlendFactor,SampleMask) (This)->OMSetBlendState(pBlendState,BlendFactor,SampleMask)
#define ID3D11DeviceContext_RSSetViewports(This,NumViewports,pViewports) (This)->RSSetViewports(NumViewports,pViewports)
#define ID3D11DeviceContext_RSSetScissorRects(This,NumRects,pRects) (This)->RSSetScissorRects(NumRects,pRects)
#define ID3D11DeviceContext_RSSetState(This,pRasterizerState) (This)->RSSetState(pRasterizerState)
#define ID3D11DeviceContext_PSSetShader(This,pPixelShader,ppClassInstances,NumClassInstances) (This)->PSSetShader(pPixelShader,ppClassInstances,NumClassInstances)
#define ID3D11DeviceContext_PSSetShaderResources(This,StartSlot,NumViews,ppShaderResourceViews) (This)->PSSetShaderResources(StartSlot,NumViews,ppShaderResourceViews)
#define ID3D11DeviceContext_PSSetSamplers(This,StartSlot,NumSamplers,ppSamplers) (This)->PSSetSamplers(StartSlot,NumSamplers,ppSamplers)
#define ID3D11DeviceContext_PSSetConstantBuffers(This,StartSlot,NumBuffers,ppConstantBuffers) (This)->PSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers)
#define ID3D11Buffer_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define ID3D11Texture2D_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define IDXGISwapChain_SetFullscreenState(This,Fullscreen,pTarget) (This)->SetFullscreenState(Fullscreen,pTarget)
#define IDXGISwapChain_ResizeBuffers(This,BufferCount,Width,Height,NewFormat,SwapChainFlags) (This)->ResizeBuffers(BufferCount,Width,Height,NewFormat,SwapChainFlags)
#define IDXGISwapChain_GetBuffer(This,Buffer,riid,ppSurface) (This)->GetBuffer(Buffer,riid,ppSurface)
#define IDXGISwapChain_Present(This,SyncInterval,Flags) (This)->Present(SyncInterval,Flags)
#define IDXGISwapChain1_SetRotation(This,Rotation) (This)->SetRotation(Rotation)
#define IDXGIAdapter_GetDesc(This,pDesc) (This)->GetDesc(pDesc)
#define IDXGIFactory_MakeWindowAssociation(This,WindowHandle,Flags) (This)->MakeWindowAssociation(WindowHandle,Flags)
#define IDXGIFactory_EnumAdapters(This,Adapter,ppAdapter) (This)->EnumAdapters(Adapter,ppAdapter)
#define IDXGIFactory_CreateSwapChain(This,pDevice,pDesc,ppSwapChain) (This)->CreateSwapChain(pDevice,pDesc,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForCoreWindow(This,pDevice,pWindow,pDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForCoreWindow(pDevice,pWindow,pDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForComposition(This,pDevice,pDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForComposition(pDevice,pDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIFactory2_CreateSwapChainForHwnd(This,pDevice,hWnd,pDesc,pFullscreenDesc,pRestrictToOutput,ppSwapChain) (This)->CreateSwapChainForHwnd(pDevice,hWnd,pDesc,pFullscreenDesc,pRestrictToOutput,ppSwapChain)
#define IDXGIDevice1_SetMaximumFrameLatency(This,MaxLatency) (This)->SetMaximumFrameLatency(MaxLatency)
#define IUnknown_Release(This) (This)->Release()
#endif /* C++ style interface */

#define SAFE_RELEASE(X) if ((X)) {IUnknown_Release(SDL_static_cast(IUnknown*, X)); X = NULL;}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif

static const GUID D3D11_IID_IDXGIFactory = {0x7B7166EC, 0x21C7, 0x44AE,{0xB2, 0x1A, 0xC9, 0xAE, 0x32, 0x1A, 0xE3, 0x69}};
static const GUID D3D11_IID_IDXGIFactory2 = {0x50C83A1C, 0xE072, 0x4C48,{0x87, 0xB0, 0x36, 0x30, 0xFA, 0x36, 0xA6, 0xD0}};
static const GUID D3D11_IID_ID3D11Device1 = {0xA04BFB29, 0x08EF, 0x43D6,{0xA4, 0x9C, 0xA9, 0xBD, 0xBD, 0xCB, 0xE6, 0x86}};
static const GUID D3D11_IID_ID3D11DeviceContext1 = {0xBB2C6FAA, 0xB5FB, 0x4082,{0x8E, 0x6B, 0x38, 0x8B, 0x8C, 0xFA, 0x90, 0xE1}};
static const GUID D3D11_IID_IDXGIDevice1 = {0x77DB970F, 0x6276, 0x48BA,{0xBA, 0x28, 0x07, 0x01, 0x43, 0xB4, 0x39, 0x2C}};
static const GUID D3D11_IID_ID3D11Texture2D = {0x6F15AAF2, 0xD208, 0x4E89,{0x9A, 0xB4, 0x48, 0x95, 0x35, 0xD3, 0x4F, 0x9C}};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern bool g_inited;

typedef HRESULT (WINAPI *PFN_CREATE_DXGI_FACTORY)(REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI *PFN_D3D11_CREATE_DEVICE)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

static const float inv255f = (1.0f / 255.0f);

#ifdef _USE_D3D_SHADER_COMPILER_
HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if(!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_OPTIMIZATION_LEVEL3;

	const D3D_SHADER_MACRO defines[] =
	{
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
	if(FAILED(hr))
	{
		if(errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			UTIL_MessageBox(true, (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if(shaderBlob)
			shaderBlob->Release();

		return hr;
	}
	*blob = shaderBlob;
	return hr;
}
#endif

SurfaceDirect3D11::SurfaceDirect3D11()
{
	g_engine.RecreateWindow(false);

	#ifndef SDL_VIDEO_DRIVER_WINRT
	m_dxgiHandle = SDL_LoadObject("dxgi.dll");
	if(m_dxgiHandle)
		m_d3d11Handle = SDL_LoadObject("d3d11.dll");
	#endif

	m_vertices.reserve(DIRECT3D11_MAX_VERTICES);
	m_sprites.reserve(MAX_SPRITES);
	m_automapTiles.reserve(MAX_AUTOMAPTILES);
}

SurfaceDirect3D11::~SurfaceDirect3D11()
{
	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseDirect3DTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BD3D11Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);
	
	for(std::vector<Direct3D11Texture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	ID3D11Buffer* indexBuffer = SDL_reinterpret_cast(ID3D11Buffer*, m_indexBuffer);
	SAFE_RELEASE(indexBuffer);
	ID3D11Buffer* vertexBuffer = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer);
	SAFE_RELEASE(vertexBuffer);
	
	IDXGISwapChain* swapChain = SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain);
	SAFE_RELEASE(swapChain);

	ID3D11RenderTargetView* mainRenderTarget = SDL_reinterpret_cast(ID3D11RenderTargetView*, m_mainRenderTargetView);
	SAFE_RELEASE(mainRenderTarget);

	ID3D11BlendState* blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendMod);
	SAFE_RELEASE(blendState);
	blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendAdd);
	SAFE_RELEASE(blendState);
	blendState = SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend);
	SAFE_RELEASE(blendState);

	ID3D11RasterizerState* rasterizer = SDL_reinterpret_cast(ID3D11RasterizerState*, m_clippedRasterizer);
	SAFE_RELEASE(rasterizer);
	rasterizer = SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer);
	SAFE_RELEASE(rasterizer);

	ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[1]);
	SAFE_RELEASE(sampler);
	sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[0]);
	SAFE_RELEASE(sampler);

	ID3D11PixelShader* pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture);
	SAFE_RELEASE(pixelShader);
	pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid);
	SAFE_RELEASE(pixelShader);
	pixelShader = SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen);
	SAFE_RELEASE(pixelShader);

	ID3D11Buffer* pixelShaderConstant = SDL_reinterpret_cast(ID3D11Buffer*, m_pixelShaderConstants);
	SAFE_RELEASE(pixelShaderConstant);
	ID3D11Buffer* vertexShaderConstant = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexShaderConstants);
	SAFE_RELEASE(vertexShaderConstant);
	ID3D11InputLayout* inputLayout = SDL_reinterpret_cast(ID3D11InputLayout*, m_inputLayout);
	SAFE_RELEASE(inputLayout);
	ID3D11VertexShader* vertexShader = SDL_reinterpret_cast(ID3D11VertexShader*, m_vertexShader);
	SAFE_RELEASE(vertexShader);
	
	IDXGIAdapter* dxgiAdapter = SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter);
	SAFE_RELEASE(dxgiAdapter);
	IDXGIFactory* dxgiFactory = SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory);
	SAFE_RELEASE(dxgiFactory);
	ID3D11Device* d3dDevice = SDL_reinterpret_cast(ID3D11Device*, m_device);
	SAFE_RELEASE(d3dDevice);
	ID3D11DeviceContext* d3dContext = SDL_reinterpret_cast(ID3D11DeviceContext*, m_context);
	SAFE_RELEASE(d3dContext);
	if(m_scaled_gameWindow)
		releaseDirect3DTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseDirect3DTexture(m_gameWindow);

	#ifndef SDL_VIDEO_DRIVER_WINRT
	if(m_d3d11Handle)
		SDL_UnloadObject(m_d3d11Handle);

	if(m_dxgiHandle)
		SDL_UnloadObject(m_dxgiHandle);
	#endif
}

bool SurfaceDirect3D11::createDirect3DTexture(Direct3D11Texture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	releaseDirect3DTexture(texture);

	D3D11_TEXTURE2D_DESC textureDesc;
	SDL_zero(textureDesc);
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	if(frameBuffer)
		textureDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET);
	else
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &textureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &texture.m_texture));
	if(FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	result = ID3D11Device_CreateShaderResourceView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_reinterpret_cast(ID3D11Resource*, texture.m_texture), &resourceViewDesc, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &texture.m_resource));
	if(FAILED(result))
	{
		releaseDirect3DTexture(texture);
		return false;
	}

	if(frameBuffer)
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		result = ID3D11Device_CreateRenderTargetView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_reinterpret_cast(ID3D11Resource*, texture.m_texture), &renderTargetViewDesc, SDL_reinterpret_cast(ID3D11RenderTargetView**, &texture.m_frameBuffer));
		if(FAILED(result))
		{
			releaseDirect3DTexture(texture);
			return false;
		}
	}

	texture.m_width = width;
	texture.m_height = height;
	texture.m_scaleW = 1.0f / width;
	texture.m_scaleH = 1.0f / height;
	texture.m_linearSample = linearSampler;
	return true;
}

bool SurfaceDirect3D11::updateTextureData(Direct3D11Texture& texture, unsigned char* data)
{
	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(SDL_reinterpret_cast(ID3D11Texture2D*, texture.m_texture), &stagingTextureDesc);
	stagingTextureDesc.Width = texture.m_width;
	stagingTextureDesc.Height = texture.m_height;
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
		return false;

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_WRITE, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, textureMemory.pData);
	UINT length = texture.m_width * 4;
	if(length == textureMemory.RowPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length * texture.m_height);
	else
	{
		if(length > textureMemory.RowPitch)
			length = textureMemory.RowPitch;
		for(Uint32 row = 0; row < texture.m_height; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, data), length);
			data += length;
			dstData += textureMemory.RowPitch;
		}
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, texture.m_texture), 0, 0, 0, 0, SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, NULL);
	SAFE_RELEASE(stagingTexture);
	return true;
}

void SurfaceDirect3D11::releaseDirect3DTexture(Direct3D11Texture& texture)
{
	if(texture.m_frameBuffer)
	{
		ID3D11RenderTargetView* frameBuffer = SDL_reinterpret_cast(ID3D11RenderTargetView*, texture.m_frameBuffer);
		SAFE_RELEASE(frameBuffer);
		texture.m_frameBuffer = NULL;
	}
	if(texture.m_resource)
	{
		ID3D11ShaderResourceView* res = SDL_reinterpret_cast(ID3D11ShaderResourceView*, texture.m_resource);
		SAFE_RELEASE(res);
		texture.m_resource = NULL;
	}
	if(texture.m_texture)
	{
		ID3D11Texture2D* tex = SDL_reinterpret_cast(ID3D11Texture2D*, texture.m_texture);
		SAFE_RELEASE(tex);
		texture.m_texture = NULL;
	}
}

void SurfaceDirect3D11::updateTextureScaling(Direct3D11Texture& texture)
{
	if(m_usingLinearSample != texture.m_linearSample)
	{
		m_usingLinearSample = texture.m_linearSample;
		ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, (m_usingLinearSample ? m_samplers[1] : m_samplers[0]));
		ID3D11DeviceContext_PSSetSamplers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, &sampler);
	}
}

#ifdef SDL_VIDEO_DRIVER_WINRT
IUnknown* D3D11_GetCoreWindowFromSDLWindow(SDL_Window* window)
{
	if(!window)
		return NULL;

	SDL_SysWMinfo sdlWindowInfo;
	SDL_VERSION(&sdlWindowInfo.version);
	if(!SDL_GetWindowWMInfo(window, &sdlWindowInfo))
		return NULL;

	if(sdlWindowInfo.subsystem != SDL_SYSWM_WINRT)
		return NULL;

	if(!sdlWindowInfo.info.winrt.window)
		return NULL;

	ABI::Windows::UI::Core::ICoreWindow* coreWindow = NULL;
	if(FAILED(sdlWindowInfo.info.winrt.window->QueryInterface(&coreWindow)))
		return NULL;

	IUnknown* coreWindowAsIUnknown = NULL;
	coreWindow->QueryInterface(&coreWindowAsIUnknown);
	coreWindow->Release();
	return coreWindowAsIUnknown;
}

static DXGI_MODE_ROTATION D3D11_GetCurrentRotation()
{
	const DisplayOrientations currentOrientation = WINRT_DISPLAY_PROPERTY(CurrentOrientation);
	switch(currentOrientation)
	{
		#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		case DisplayOrientations::Landscape:
			return DXGI_MODE_ROTATION_ROTATE90;
		case DisplayOrientations::Portrait:
			return DXGI_MODE_ROTATION_IDENTITY;
		case DisplayOrientations::LandscapeFlipped:
			return DXGI_MODE_ROTATION_ROTATE270;
		case DisplayOrientations::PortraitFlipped:
			return DXGI_MODE_ROTATION_ROTATE180;
		#else
		case DisplayOrientations::Landscape:
			return DXGI_MODE_ROTATION_IDENTITY;
		case DisplayOrientations::Portrait:
			return DXGI_MODE_ROTATION_ROTATE270;
		case DisplayOrientations::LandscapeFlipped:
			return DXGI_MODE_ROTATION_ROTATE180;
		case DisplayOrientations::PortraitFlipped:
			return DXGI_MODE_ROTATION_ROTATE90;
		#endif
		default: return DXGI_MODE_ROTATION_IDENTITY;
	}
}
#else
static DXGI_MODE_ROTATION D3D11_GetCurrentRotation()
{
	return DXGI_MODE_ROTATION_IDENTITY;
}
#endif

static bool D3D11_IsDisplayRotated90Degrees(DXGI_MODE_ROTATION rotation)
{
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		case DXGI_MODE_ROTATION_ROTATE270:
			return true;
		default: return false;
	}
}

HRESULT SurfaceDirect3D11::createSwapChain(int w, int h)
{
	if(m_useOldDXGIinterface)
	{
		#ifdef SDL_VIDEO_DRIVER_WINDOWS
		SDL_SysWMinfo windowinfo;
		SDL_VERSION(&windowinfo.version);
		SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

		SDL_DisplayMode fullscreen_mode;
		//Uint32 window_flags = SDL_GetWindowFlags(g_engine.m_window);
		SDL_GetWindowDisplayMode(g_engine.m_window, &fullscreen_mode);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = w;
		swapChainDesc.BufferDesc.Height = h;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = fullscreen_mode.refresh_rate;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = windowinfo.info.win.window;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
		swapChainDesc.Flags = 0;

		HRESULT result = IDXGIFactory_CreateSwapChain(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), &swapChainDesc, SDL_reinterpret_cast(IDXGISwapChain**, &m_swapChain));
		if(FAILED(result))
			return result;

		/*For some reason making the swapchain going fullscreen state make the whole system unresponsible so I disabled it
		It doesn't matter if I use SetFullscreenState or the Windowed variable in swapchain creation
		if(window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			IDXGISwapChain_SetFullscreenState(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), TRUE, NULL);
		}*/
		IDXGIFactory_MakeWindowAssociation(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), windowinfo.info.win.window, DXGI_MWA_NO_WINDOW_CHANGES);
		return S_OK;
		#else
		return E_NOINTERFACE;
		#endif
	}

	#ifdef SDL_VIDEO_DRIVER_WINRT
	IUnknown* coreWindow = D3D11_GetCoreWindowFromSDLWindow(g_engine.m_window);
	const bool usingXAML = (coreWindow == NULL);
	#else
	IUnknown* coreWindow = NULL;
	const bool usingXAML = false;
	#endif

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = w;
	swapChainDesc.Height = h;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	#else
	if(usingXAML)
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	else
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	#endif
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	if(coreWindow)
	{
		HRESULT result = IDXGIFactory2_CreateSwapChainForCoreWindow(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), coreWindow, &swapChainDesc, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;
	}
	else if(usingXAML)
	{
		HRESULT result = IDXGIFactory2_CreateSwapChainForComposition(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), &swapChainDesc, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;

		#if WINAPI_FAMILY == WINAPI_FAMILY_APP
		result = ISwapChainBackgroundPanelNative_SetSwapChain(WINRT_GlobalSwapChainBackgroundPanelNative, SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain));
		if(FAILED(result))
			return result;
		#else
		return E_NOINTERFACE;
		#endif
	}
	else
	{
		#ifdef SDL_VIDEO_DRIVER_WINDOWS
		SDL_SysWMinfo windowinfo;
		SDL_VERSION(&windowinfo.version);
		SDL_GetWindowWMInfo(g_engine.m_window, &windowinfo);

		HRESULT result = IDXGIFactory2_CreateSwapChainForHwnd(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), SDL_reinterpret_cast(IUnknown*, m_device), windowinfo.info.win.window, &swapChainDesc, NULL, NULL, SDL_reinterpret_cast(IDXGISwapChain1**, &m_swapChain));
		if(FAILED(result))
			return result;
		IDXGIFactory_MakeWindowAssociation(SDL_reinterpret_cast(IDXGIFactory2*, m_dxgiFactory), windowinfo.info.win.window, DXGI_MWA_NO_WINDOW_CHANGES);
		#else
		return E_NOINTERFACE;
		#endif
	}
	SAFE_RELEASE(coreWindow);
	return S_OK;
}

void SurfaceDirect3D11::createWindowSizeDependentResources()
{
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, NULL, NULL);

	ID3D11RenderTargetView* mainRenderTarget = SDL_reinterpret_cast(ID3D11RenderTargetView*, m_mainRenderTargetView);
	SAFE_RELEASE(mainRenderTarget);

	int w_width, w_height;
	SDL_GetWindowSize(g_engine.m_window, &w_width, &w_height);

	DXGI_MODE_ROTATION rotation = D3D11_GetCurrentRotation();
	if(D3D11_IsDisplayRotated90Degrees(rotation))
	{
		int tmp = w_width;
		w_width = w_height;
		w_height = tmp;
	}

	HRESULT result;
	if(m_swapChain)
	{
		#if !defined(SDL_VIDEO_DRIVER_WINRT) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
		result = IDXGISwapChain_ResizeBuffers(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, w_width, w_height, DXGI_FORMAT_UNKNOWN, 0);
		if(result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_REMOVED)
		{
			//g_inited = false should recreate the whole d3d11 interface in next frame
			g_inited = false;
			return;
		}
		else if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to resize swapchain.");
			exit(-1);
		}
		#endif
	}
	else
	{
		result = createSwapChain(w_width, w_height);
		if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to create swapchain.");
			exit(-1);
		}
	}

	#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
	if(!m_useOldDXGIinterface)
	{
		result = IDXGISwapChain1_SetRotation(SDL_reinterpret_cast(IDXGISwapChain1*, m_swapChain), rotation);
		if(FAILED(result))
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to set rotation.");
			exit(-1);
		}
	}
	#endif

	ID3D11Texture2D* backBuffer;
	result = IDXGISwapChain_GetBuffer(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, IID_ID3D11Texture2D, SDL_reinterpret_cast(void**, &backBuffer));
	if(FAILED(result))
	{
		UTIL_MessageBox(true, "Direct3D11: Failed to get backBuffer.");
		exit(-1);
	}
	
	result = ID3D11Device_CreateRenderTargetView(SDL_reinterpret_cast(ID3D11Device*, m_device), SDL_static_cast(ID3D11Resource*, backBuffer), NULL, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView));
	if(FAILED(result))
	{
		SAFE_RELEASE(backBuffer);
		UTIL_MessageBox(true, "Direct3D11: Failed to create backBuffer frameBuffer.");
		exit(-1);
	}
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
	SAFE_RELEASE(backBuffer);
	m_needReset = false;
}

void SurfaceDirect3D11::updateViewport()
{
	if(m_viewPortW == 0 || m_viewPortH == 0)
		return;

	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if(!m_currentRenderTargetView || m_currentRenderTargetView == m_mainRenderTargetView)
		rotation = D3D11_GetCurrentRotation();

	float p_11, p_12, p_13, p_14;
	float p_21, p_22, p_23, p_24;
	float p_31, p_32, p_33, p_34;
	float p_41, p_42, p_43, p_44;
	switch(rotation)
	{
		//Calculate projection
		case DXGI_MODE_ROTATION_IDENTITY:
		{
			p_11 = p_22 = p_33 = p_44 = 1.0f;
			p_12 = p_21 = p_32 = p_43 = 0.0f;
			p_13 = p_24 = p_31 = p_42 = 0.0f;
			p_14 = p_23 = p_34 = p_41 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			float r = SDL_static_cast(float, M_PI * 0.5f);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			float r = SDL_static_cast(float, M_PI);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			float r = SDL_static_cast(float, -M_PI * 0.5f);
			float sinR = SDL_sinf(r);
			float cosR = SDL_cosf(r);

			p_11 = p_22 = cosR;
			p_12 = sinR; p_21 = -sinR;
			p_13 = p_14 = 0.0f;
			p_23 = p_24 = 0.0f;
			p_31 = p_32 = p_34 = 0.0f;
			p_33 = p_44 = 1.0f;
			p_41 = p_42 = p_43 = 0.0f;
		}
		break;
		default: return;
	}

	//Calculate view
	float v_11, v_12, v_13, v_14;
	float v_21, v_22, v_23, v_24;
	float v_31, v_32, v_33, v_34;
	float v_41, v_42, v_43, v_44;
	v_11 = 2.0f / m_viewPortW; v_12 = 0.0f; v_13 = 0.0f; v_14 = 0.0f;
	v_21 = 0.0f; v_22 = -2.0f / m_viewPortH; v_23 = 0.0f; v_24 = 0.0f;
	v_31 = 0.0f; v_32 = 0.0f; v_33 = 1.0f; v_34 = 0.0f;
	v_41 = -1.0f; v_42 = 1.0f; v_43 = 0.0f; v_44 = 1.0f;

	//Calculate projection and view
	m_vertexShaderConstantsData.pv_11 = v_11 * p_11 + v_12 * p_21 + v_13 * p_31 + v_14 * p_41;
	m_vertexShaderConstantsData.pv_12 = v_11 * p_12 + v_12 * p_22 + v_13 * p_32 + v_14 * p_42;
	m_vertexShaderConstantsData.pv_13 = v_11 * p_13 + v_12 * p_23 + v_13 * p_33 + v_14 * p_43;
	m_vertexShaderConstantsData.pv_14 = v_11 * p_14 + v_12 * p_24 + v_13 * p_34 + v_14 * p_44;
	m_vertexShaderConstantsData.pv_21 = v_21 * p_11 + v_22 * p_21 + v_23 * p_31 + v_24 * p_41;
	m_vertexShaderConstantsData.pv_22 = v_21 * p_12 + v_22 * p_22 + v_23 * p_32 + v_24 * p_42;
	m_vertexShaderConstantsData.pv_23 = v_21 * p_13 + v_22 * p_23 + v_23 * p_33 + v_24 * p_43;
	m_vertexShaderConstantsData.pv_24 = v_21 * p_14 + v_22 * p_24 + v_23 * p_34 + v_24 * p_44;
	m_vertexShaderConstantsData.pv_31 = v_31 * p_11 + v_32 * p_21 + v_33 * p_31 + v_34 * p_41;
	m_vertexShaderConstantsData.pv_32 = v_31 * p_12 + v_32 * p_22 + v_33 * p_32 + v_34 * p_42;
	m_vertexShaderConstantsData.pv_33 = v_31 * p_13 + v_32 * p_23 + v_33 * p_33 + v_34 * p_43;
	m_vertexShaderConstantsData.pv_34 = v_31 * p_14 + v_32 * p_24 + v_33 * p_34 + v_34 * p_44;
	m_vertexShaderConstantsData.pv_41 = v_41 * p_11 + v_42 * p_21 + v_43 * p_31 + v_44 * p_41;
	m_vertexShaderConstantsData.pv_42 = v_41 * p_12 + v_42 * p_22 + v_43 * p_32 + v_44 * p_42;
	m_vertexShaderConstantsData.pv_43 = v_41 * p_13 + v_42 * p_23 + v_43 * p_33 + v_44 * p_43;
	m_vertexShaderConstantsData.pv_44 = v_41 * p_14 + v_42 * p_24 + v_43 * p_34 + v_44 * p_44;
	ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexShaderConstants), 0, NULL, &m_vertexShaderConstantsData, 0, 0);

	D3D11_VIEWPORT viewport;
	if(D3D11_IsDisplayRotated90Degrees(rotation))
	{
		viewport.TopLeftX = SDL_static_cast(float, m_viewPortY);
		viewport.TopLeftY = SDL_static_cast(float, m_viewPortX);
		viewport.Width = SDL_static_cast(float, m_viewPortH);
		viewport.Height = SDL_static_cast(float, m_viewPortW);
	}
	else
	{
		viewport.TopLeftX = SDL_static_cast(float, m_viewPortX);
		viewport.TopLeftY = SDL_static_cast(float, m_viewPortY);
		viewport.Width = SDL_static_cast(float, m_viewPortW);
		viewport.Height = SDL_static_cast(float, m_viewPortH);
	}
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	ID3D11DeviceContext_RSSetViewports(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, &viewport);
}

bool SurfaceDirect3D11::updateVertexBuffer(const void* vertexData, size_t dataSize)
{
	if(m_vertexBuffer && m_vertexBufferSize >= dataSize)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexBuffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if(FAILED(result))
			return false;

		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, mappedResource.pData), SDL_reinterpret_cast(const Uint8*, vertexData), dataSize);
		ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_vertexBuffer), 0);
	}
	else
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		if(m_vertexBuffer)
			ID3D11Buffer_GetDesc(SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer), &vertexBufferDesc);
		else
			SDL_zero(vertexBufferDesc);

		ID3D11Buffer* vertexBuffer = SDL_reinterpret_cast(ID3D11Buffer*, m_vertexBuffer);
		SAFE_RELEASE(vertexBuffer);

		vertexBufferDesc.ByteWidth = SDL_static_cast(UINT, dataSize);
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		SDL_zero(vertexBufferData);
		vertexBufferData.pSysMem = vertexData;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		HRESULT result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &vertexBufferDesc, &vertexBufferData, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexBuffer));
		if(FAILED(result))
			return false;

		const UINT stride = sizeof(VertexD3D11);
		const UINT offset = 0;
		ID3D11DeviceContext_IASetVertexBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexBuffer), &stride, &offset);
		m_vertexBufferSize = dataSize;
	}
	return true;
}

bool SurfaceDirect3D11::createVertexShader(void* d3dDevice, void** vertexShader, void** inputLayout)
{
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	
	HRESULT result = ID3D11Device_CreateVertexShader(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), D3D11_VertexShader, sizeof(D3D11_VertexShader), NULL, SDL_reinterpret_cast(ID3D11VertexShader**, vertexShader));
	if(FAILED(result))
		 return false;
	result = ID3D11Device_CreateInputLayout(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), vertexDesc, ARRAYSIZE(vertexDesc), D3D11_VertexShader, sizeof(D3D11_VertexShader), SDL_reinterpret_cast(ID3D11InputLayout**, inputLayout));
	if(FAILED(result))
		return false;
	return true;
}

bool SurfaceDirect3D11::createPixelShader(void* d3dDevice, const void* shader_data, size_t shader_size, void** pixelShader)
{
	HRESULT result = ID3D11Device_CreatePixelShader(SDL_reinterpret_cast(ID3D11Device*, d3dDevice), shader_data, shader_size, NULL, SDL_reinterpret_cast(ID3D11PixelShader**, pixelShader));
	if(FAILED(result))
		return false;
	return true;
}

bool SurfaceDirect3D11::isSupported()
{
	PFN_CREATE_DXGI_FACTORY CreateDXGIFactoryFunc;
	PFN_D3D11_CREATE_DEVICE D3D11CreateDeviceFunc;

	#ifdef SDL_VIDEO_DRIVER_WINRT
	CreateDXGIFactoryFunc = CreateDXGIFactory1;
	D3D11CreateDeviceFunc = D3D11CreateDevice;
	#else
	if(!m_dxgiHandle || !m_d3d11Handle)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create Direct3D11 interface.");
		return false;
	}

	CreateDXGIFactoryFunc = SDL_reinterpret_cast(PFN_CREATE_DXGI_FACTORY, SDL_LoadFunction(m_dxgiHandle, "CreateDXGIFactory"));
	if(!CreateDXGIFactoryFunc)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to fetch 'CreateDXGIFactory' function.");
		return false;
	}

	D3D11CreateDeviceFunc = SDL_reinterpret_cast(PFN_D3D11_CREATE_DEVICE, SDL_LoadFunction(m_d3d11Handle, "D3D11CreateDevice"));
	if(!D3D11CreateDeviceFunc)
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to fetch 'D3D11CreateDevice' function.");
		return false;
	}
	#endif
	
	HRESULT result = CreateDXGIFactoryFunc(D3D11_IID_IDXGIFactory2, &m_dxgiFactory);
	if(FAILED(result))
	{
		m_useOldDXGIinterface = true;
		result = CreateDXGIFactoryFunc(D3D11_IID_IDXGIFactory, &m_dxgiFactory);
		if(FAILED(result))
		{
			UTIL_MessageBox(false, "Direct3D11: Failed to create DXGI Factory.");
			return false;
		}
	}

	result = IDXGIFactory_EnumAdapters(SDL_reinterpret_cast(IDXGIFactory*, m_dxgiFactory), 0, SDL_reinterpret_cast(IDXGIAdapter**, &m_dxgiAdapter));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to get device adapter.");
		return false;
	}

	D3D_FEATURE_LEVEL featureLevel_11_1 = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL featureLevel;
	result = D3D11CreateDeviceFunc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), D3D_DRIVER_TYPE_UNKNOWN, NULL, (D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_BGRA_SUPPORT), &featureLevel_11_1, 1, D3D11_SDK_VERSION, SDL_reinterpret_cast(ID3D11Device**, &m_device), &featureLevel, SDL_reinterpret_cast(ID3D11DeviceContext**, &m_context));
	if(FAILED(result))
	{
		result = D3D11CreateDeviceFunc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), D3D_DRIVER_TYPE_UNKNOWN, NULL, (D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_BGRA_SUPPORT), featureLevels, SDL_arraysize(featureLevels), D3D11_SDK_VERSION, SDL_reinterpret_cast(ID3D11Device**, &m_device), &featureLevel, SDL_reinterpret_cast(ID3D11DeviceContext**, &m_context));
		if(FAILED(result))
		{
			UTIL_MessageBox(false, "Direct3D11: Failed to create device.");
			return false;
		}
	}

	void* d3device1;
	result = ID3D11Device_QueryInterface(SDL_reinterpret_cast(ID3D11Device*, m_device), D3D11_IID_ID3D11Device1, &d3device1);
	if(SUCCEEDED(result))
	{
		ID3D11Device* device = SDL_reinterpret_cast(ID3D11Device*, m_device);
		SAFE_RELEASE(device);
		m_device = d3device1;
	}

	void* d3dContext1;
	result = ID3D11DeviceContext_QueryInterface(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_IID_ID3D11DeviceContext1, &d3dContext1);
	if(SUCCEEDED(result))
	{
		ID3D11DeviceContext* context = SDL_reinterpret_cast(ID3D11DeviceContext*, m_context);
		SAFE_RELEASE(context);
		m_context = d3dContext1;
	}
	
	IDXGIDevice1* dxgiDevice;
	result = ID3D11Device_QueryInterface(SDL_reinterpret_cast(ID3D11Device*, m_device), D3D11_IID_IDXGIDevice1, SDL_reinterpret_cast(void**, &dxgiDevice));
	if(SUCCEEDED(result))
	{
		IDXGIDevice1_SetMaximumFrameLatency(dxgiDevice, 1);
		SAFE_RELEASE(dxgiDevice);
	}

	switch(featureLevel)
	{
		case D3D_FEATURE_LEVEL_11_1:
		{
			m_software = "Direct3D 11.1";
			m_maxTextureSize = 16384;
		}
		break;
		case D3D_FEATURE_LEVEL_11_0:
		{
			m_software = "Direct3D 11.0";
			m_maxTextureSize = 16384;
		}
		break;
		case D3D_FEATURE_LEVEL_10_1:
		{
			m_software = "Direct3D 10.1";
			m_maxTextureSize = 8192;
		}
		break;
		case D3D_FEATURE_LEVEL_10_0:
		{
			m_software = "Direct3D 10.0";
			m_maxTextureSize = 8192;
		}
		break;
		case D3D_FEATURE_LEVEL_9_3:
		{
			m_software = "Direct3D 9.0c";
			m_maxTextureSize = 4096;
		}
		break;
		case D3D_FEATURE_LEVEL_9_2:
		{
			m_software = "Direct3D 9.0b";
			m_maxTextureSize = 2048;
		}
		break;
		case D3D_FEATURE_LEVEL_9_1:
		{
			m_software = "Direct3D 9.0";
			m_maxTextureSize = 2048;
		}
		break;
		default:
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Direct3D11: Unexpected feature level: %d", featureLevel);
			UTIL_MessageBox(false, g_buffer);
			return false;
		}
	}

	if(!createVertexShader(m_device, &m_vertexShader, &m_inputLayout))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create vertex shader.");
		return false;
	}

	if(!createPixelShader(m_device, D3D11_PixelShader_Solid, sizeof(D3D11_PixelShader_Solid), &m_pixelShaderSolid))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader(SOLID COLORS).");
		return false;
	}

	if(!createPixelShader(m_device, D3D11_PixelShader_Texture, sizeof(D3D11_PixelShader_Texture), &m_pixelShaderTexture))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader(RGBA TEXTURES).");
		return false;
	}
	
	if(createPixelShader(m_device, D3D11_sharpen, sizeof(D3D11_sharpen), &m_pixelShaderSharpen))
		m_haveSharpening = true;
	
	D3D11_BUFFER_DESC constantBufferDesc;
	SDL_zero(constantBufferDesc);
	constantBufferDesc.ByteWidth = sizeof(VertexShaderConstants);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &constantBufferDesc, NULL, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create pixel shader constants.");
		return false;
	}
	result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &constantBufferDesc, NULL, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexShaderConstants));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create vertex shader constants.");
		return false;
	}
	
	D3D11_SAMPLER_DESC samplerDesc;
	SDL_zero(samplerDesc);
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	result = ID3D11Device_CreateSamplerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &samplerDesc, SDL_reinterpret_cast(ID3D11SamplerState**, &m_samplers[0]));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create texture sampler(NEAREST).");
		return false;
	}
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = ID3D11Device_CreateSamplerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &samplerDesc, SDL_reinterpret_cast(ID3D11SamplerState**, &m_samplers[1]));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create texture sampler(LINEAR).");
		return false;
	}

	D3D11_RASTERIZER_DESC rasterDesc;
	SDL_zero(rasterDesc);
	rasterDesc.AntialiasedLineEnable = FALSE;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	result = ID3D11Device_CreateRasterizerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &rasterDesc, SDL_reinterpret_cast(ID3D11RasterizerState**, &m_mainRasterizer));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create rasterizer(MAIN).");
		return false;
	}
	rasterDesc.ScissorEnable = TRUE;
	result = ID3D11Device_CreateRasterizerState(SDL_reinterpret_cast(ID3D11Device*, m_device), &rasterDesc, SDL_reinterpret_cast(ID3D11RasterizerState**, &m_clippedRasterizer));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create rasterizer(CLIPPED).");
		return false;
	}

	D3D11_BLEND_DESC blendDesc;
	SDL_zero(blendDesc);
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendBlend));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendAdd));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	result = ID3D11Device_CreateBlendState(SDL_reinterpret_cast(ID3D11Device*, m_device), &blendDesc, SDL_reinterpret_cast(ID3D11BlendState**, &m_blendMod));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Failed to create graphic pipeline.");
		return false;
	}

	ID3D11DeviceContext_IASetInputLayout(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11InputLayout*, m_inputLayout));
	ID3D11DeviceContext_VSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11VertexShader*, m_vertexShader), NULL, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_vertexShaderConstants));
	
	#ifdef _USE_D3D_SHADER_COMPILER_
	ID3DBlob* vsBlob = nullptr;
	HRESULT hr = CompileShader(L"DX11_sharpen.hlsl", "main", "ps_4_0_level_9_3", &vsBlob);
	if(SUCCEEDED(hr))
	{
		SDL_RWops* src = SDL_RWFromFile("test.ini", "wb");
		if(src)
		{
			Uint32* data = SDL_reinterpret_cast(Uint32*, vsBlob->GetBufferPointer());
			size_t len = vsBlob->GetBufferSize();
			bool started = false;
			size_t counter = 0;
			for(size_t i = 0; i < len; i += 4)
			{
				if(++counter == 6)
				{
					if(started)
					{
						started = false;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), ", 0x%08x,\n", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					else
					{
						started = true;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), "0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					counter = 0;
				}
				else
				{
					if(started)
					{
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), ", 0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
					else
					{
						started = true;
						int texSize = SDL_snprintf(g_buffer, sizeof(g_buffer), "0x%08x", *data);
						SDL_RWwrite(src, g_buffer, 1, texSize);
					}
				}
				++data;
			}
			SDL_RWclose(src);
		}
		SAFE_RELEASE(vsBlob);
	}
	#endif
	DXGI_ADAPTER_DESC adapterDesc;
	IDXGIAdapter_GetDesc(SDL_reinterpret_cast(IDXGIAdapter*, m_dxgiAdapter), &adapterDesc);
	sprintf_s(g_buffer, "%ws", adapterDesc.Description);
	m_hardware = SDL_strdup(g_buffer);
	m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, adapterDesc.DedicatedVideoMemory / 1048576));
	return true;
}

void SurfaceDirect3D11::init()
{
	m_pictures = SDL_reinterpret_cast(Direct3D11Texture*, SDL_calloc(g_pictureCounts, sizeof(Direct3D11Texture)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	if(!createDirect3DTexture(m_gameWindow, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true))
	{
		UTIL_MessageBox(false, "Direct3D11: Out of video memory.");
		exit(-1);
	}

	#if OPENGL_CORE_USE_UINT_INDICES > 0
	std::vector<Uint32> indices(DIRECT3D11_MAX_INDICES);
	Uint32 offset = 0;
	#else
	std::vector<Uint16> indices(DIRECT3D11_MAX_INDICES);
	Uint16 offset = 0;
	#endif
	for(Sint32 i = 0; i < DIRECT3D11_MAX_INDICES; i += 6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 3 + offset;
		indices[i + 4] = 2 + offset;
		indices[i + 5] = 1 + offset;

		offset += 4;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	SDL_zero(indexBufferDesc);
	#if DIRECT3D11_USE_UINT_INDICES > 0
	indexBufferDesc.ByteWidth = SDL_static_cast(UINT, DIRECT3D11_MAX_INDICES * 4);
	#else
	indexBufferDesc.ByteWidth = SDL_static_cast(UINT, DIRECT3D11_MAX_INDICES * 2);
	#endif
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	SDL_zero(indexBufferData);
	indexBufferData.pSysMem = &indices[0];
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	HRESULT result = ID3D11Device_CreateBuffer(SDL_reinterpret_cast(ID3D11Device*, m_device), &indexBufferDesc, &indexBufferData, SDL_reinterpret_cast(ID3D11Buffer**, &m_indexBuffer));
	if(FAILED(result))
	{
		UTIL_MessageBox(false, "Direct3D11: Out of video memory.");
		exit(-1);
	}

	#if DIRECT3D11_USE_UINT_INDICES > 0
	ID3D11DeviceContext_IASetIndexBuffer(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Buffer*, m_indexBuffer), DXGI_FORMAT_R32_UINT, 0);
	#else
	ID3D11DeviceContext_IASetIndexBuffer(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Buffer*, m_indexBuffer), DXGI_FORMAT_R16_UINT, 0);
	#endif
}

void SurfaceDirect3D11::doResize(Sint32, Sint32)
{
	m_needReset = true;
}

void SurfaceDirect3D11::generateSpriteAtlases()
{
	if(m_maxTextureSize >= 16384 && MAX_SPRITES > 65536)
	{
		m_spriteAtlases = (MAX_SPRITES + 262143) / 262144;
		m_spritesPerAtlas = 262144;
		m_spritesPerModulo = 16384;
	}
	else if(m_maxTextureSize >= 8192 && MAX_SPRITES > 16384)
	{
		m_spriteAtlases = (MAX_SPRITES + 65535) / 65536;
		m_spritesPerAtlas = 65536;
		m_spritesPerModulo = 8192;
	}
	else if(m_maxTextureSize >= 4096 && MAX_SPRITES > 4096)
	{
		m_spriteAtlases = (MAX_SPRITES + 16383) / 16384;
		m_spritesPerAtlas = 16384;
		m_spritesPerModulo = 4096;
	}
	else
	{
		m_spriteAtlases = (MAX_SPRITES + 4095) / 4096;
		m_spritesPerAtlas = 4096;
		m_spritesPerModulo = 2048;
	}

	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		m_spritesAtlas.emplace_back();
		if(!createDirect3DTexture(m_spritesAtlas.back(), m_spritesPerModulo, m_spritesPerModulo, false))
		{
			UTIL_MessageBox(true, "Direct3D11: Out of video memory.");
			exit(-1);
		}
	}
}

void SurfaceDirect3D11::spriteManagerReset()
{
	for(U32BD3D11Textures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseDirect3DTexture(it->second);
	
	for(std::vector<Direct3D11Texture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseDirect3DTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	generateSpriteAtlases();
}

unsigned char* SurfaceDirect3D11::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(m_needReset)
		return NULL;

	ID3D11Texture2D* backBuffer;
	HRESULT result = IDXGISwapChain_GetBuffer(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), 0, IID_ID3D11Texture2D, SDL_reinterpret_cast(void**, &backBuffer));
	if(FAILED(result))
		return NULL;
	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(backBuffer, &stagingTextureDesc);
	width = SDL_static_cast(Sint32, stagingTextureDesc.Width);
	height = SDL_static_cast(Sint32, stagingTextureDesc.Height);
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width * height * 4));
	if(!pixels)
	{
		SAFE_RELEASE(backBuffer);
		return NULL;
	}
	result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
	{
		SAFE_RELEASE(backBuffer);
		SDL_free(pixels);
		return NULL;
	}
	DXGI_MODE_ROTATION rotation = D3D11_GetCurrentRotation();
	D3D11_BOX srcBox;
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			srcBox.left = m_viewPortH - height;
			srcBox.right = m_viewPortH;
			srcBox.top = 0;
			srcBox.bottom = height;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			srcBox.left = m_viewPortW - width;
			srcBox.right = m_viewPortW;
			srcBox.top = m_viewPortH - height;
			srcBox.bottom = m_viewPortH;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			srcBox.left = 0;
			srcBox.right = height;
			srcBox.top = m_viewPortW - width;
			srcBox.bottom = m_viewPortW;
		}
		break;
		default:
		{
			srcBox.left = m_viewPortX;
			srcBox.right = m_viewPortX + width;
			srcBox.top = m_viewPortY;
			srcBox.bottom = m_viewPortY + height;
		}
		break;
	}
	srcBox.front = 0;
	srcBox.back = 1;
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, 0, 0, 0, SDL_reinterpret_cast(ID3D11Resource*, backBuffer), 0, &srcBox);
	SAFE_RELEASE(backBuffer);

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_READ, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return false;
	}
	if(stagingTextureDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || stagingTextureDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, textureMemory.pData), SDL_static_cast(size_t, width * height * 4));
		bgra = true;
	}
	else
	{
		ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return NULL;
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	SAFE_RELEASE(stagingTexture);
	return pixels;
}

void SurfaceDirect3D11::beginScene()
{
	m_spriteChecker = 0;
	++m_currentFrame;
	if(m_needReset)
	{
		m_currentRenderTargetView = NULL;
		createWindowSizeDependentResources();
		m_currentRenderTargetView = m_mainRenderTargetView;
	}

	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	if(m_currentRenderTargetView != m_mainRenderTargetView)
	{
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = g_engine.getWindowWidth();
		m_viewPortH = g_engine.getWindowHeight();
		m_currentRenderTargetView = m_mainRenderTargetView;
		updateViewport();
	}

	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer));
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);

	ID3D11SamplerState* sampler = SDL_reinterpret_cast(ID3D11SamplerState*, m_samplers[0]);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
	ID3D11DeviceContext_PSSetSamplers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, &sampler);
	m_usingLinearSample = false;
}

void SurfaceDirect3D11::endScene()
{
	scheduleBatch();

	UINT syncInterval;
	UINT presentFlags;
	#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	syncInterval = 1;
	presentFlags = 0;
	#else
	if(g_engine.isVsync())
	{
		syncInterval = 1;
		presentFlags = 0;
	}
	else
	{
		syncInterval = 0;
		presentFlags = 0;
	}
	#endif

	HRESULT result = IDXGISwapChain_Present(SDL_reinterpret_cast(IDXGISwapChain*, m_swapChain), syncInterval, presentFlags);
	if(FAILED(result))
	{
		if(result == DXGI_ERROR_DEVICE_RESET || result == DXGI_ERROR_DEVICE_REMOVED)
		{
			//g_inited = false should recreate the whole d3d11 interface in next frame
			g_inited = false;
		}
		else if(result == DXGI_ERROR_INVALID_CALL)
		{
			//Schedule swapchain reset
			m_needReset = true;
		}
		else
		{
			UTIL_MessageBox(true, "Direct3D11: Failed to present swapchain.");
			exit(-1);
		}
	}
}

Direct3D11Texture* SurfaceDirect3D11::getTextureIndex(Direct3D11Texture* texture)
{
	if(texture != m_binded_texture)
	{
		scheduleBatch();
		if(!m_binded_texture)
			ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);

		m_binded_texture = texture;
	}
	return m_binded_texture;
}

void SurfaceDirect3D11::drawQuad(Direct3D11Texture*, float vertices[8], float texcoords[8])
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], 0xFFFFFFFF);
	m_cachedVertices += 6;
}

void SurfaceDirect3D11::drawQuad(Direct3D11Texture*, float vertices[8], float texcoords[8], DWORD color)
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], color);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], color);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], color);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], color);
	m_cachedVertices += 6;
}

void SurfaceDirect3D11::scheduleBatch()
{
	if(m_cachedVertices > 0)
	{
		if(m_binded_texture)
		{
			updateTextureScaling(*m_binded_texture);
			ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11ShaderResourceView**, &m_binded_texture->m_resource));
		}
		else
		{
			ID3D11ShaderResourceView* resources = NULL;
			ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
		}

		if(!updateVertexBuffer(&m_vertices[0], m_vertices.size() * sizeof(VertexD3D11)))
		{
			m_vertices.clear();
			m_cachedVertices = 0;
			return;
		}
		
		ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if(m_cachedVertices > DIRECT3D11_MAX_INDICES)
		{
			Uint32 vertexIndex = 0;
			do
			{
				Uint32 passIndices = UTIL_min<Uint32>(m_cachedVertices, DIRECT3D11_MAX_INDICES);
				ID3D11DeviceContext_DrawIndexed(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), passIndices, 0, vertexIndex);
				m_cachedVertices -= passIndices;
				vertexIndex += DIRECT3D11_MAX_VERTICES;
			} while(m_cachedVertices > 0);
		}
		else
			ID3D11DeviceContext_DrawIndexed(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), m_cachedVertices, 0, 0);

		m_vertices.clear();
		m_cachedVertices = 0;
	}
}

bool SurfaceDirect3D11::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width < width || m_integer_scaling_height < height || !m_scaled_gameWindow)
	{
		if(!createDirect3DTexture(m_scaled_gameWindow, width, height, true, true))
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	scheduleBatch();
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_scaled_gameWindow.m_frameBuffer), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow.m_width;
	m_viewPortH = m_scaled_gameWindow.m_height;
	m_currentRenderTargetView = m_scaled_gameWindow.m_frameBuffer;
	updateViewport();

	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = sx * m_gameWindow.m_scaleW;
	float maxu = (sx + sw) * m_gameWindow.m_scaleW;
	float minv = sy * m_gameWindow.m_scaleH;
	float maxv = (sy + sh) * m_gameWindow.m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), NULL, 0, 0xFFFFFFFF);
	drawQuad(getTextureIndex(&m_gameWindow), vertices, texcoords);
	scheduleBatch();

	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	m_currentRenderTargetView = m_mainRenderTargetView;
	updateViewport();

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x + w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y + h);

	minu = 0.0f;
	maxu = width * m_scaled_gameWindow.m_scaleW;
	minv = 0.0f;
	maxv = height * m_scaled_gameWindow.m_scaleH;

	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w * (sw * m_gameWindow.m_scaleW), 1.0f / h * (sh * m_gameWindow.m_scaleH)};
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
		ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_pixelShaderConstants), 0, NULL, &textureDimensions, 0, 0);
	}
	drawQuad(getTextureIndex(&m_scaled_gameWindow), vertices, texcoords);
	scheduleBatch();
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		ID3D11Buffer* resources = NULL;
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
	}
	return true;
}

void SurfaceDirect3D11::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Uint8 antialiasing = g_engine.getAntialiasing();
	if(antialiasing == CLIENT_ANTIALIASING_INTEGER)
	{
		if(integer_scaling(sx, sy, sw, sh, x, y, w, h))
			return;
		else
		{
			antialiasing = CLIENT_ANTIALIASING_NORMAL;
			g_engine.setAntialiasing(antialiasing);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * m_gameWindow.m_scaleW;
	float maxu = (sx + sw) * m_gameWindow.m_scaleW;
	float minv = sy * m_gameWindow.m_scaleH;
	float maxv = (sy + sh) * m_gameWindow.m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	scheduleBatch();
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), NULL, 0, 0xFFFFFFFF);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		float textureDimensions[2] = {1.0f / w * (sw * m_gameWindow.m_scaleW), 1.0f / h * (sh * m_gameWindow.m_scaleH)};
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSharpen), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 1, SDL_reinterpret_cast(ID3D11Buffer**, &m_pixelShaderConstants));
		ID3D11DeviceContext_UpdateSubresource(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, m_pixelShaderConstants), 0, NULL, &textureDimensions, 0, 0);
	}

	drawQuad(getTextureIndex(&m_gameWindow), vertices, texcoords);
	scheduleBatch();
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		ID3D11Buffer* resources = NULL;
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
		ID3D11DeviceContext_PSSetConstantBuffers(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);
	}
}

void SurfaceDirect3D11::beginGameScene()
{
	scheduleBatch();
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_gameWindow.m_frameBuffer), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow.m_width;
	m_viewPortH = m_gameWindow.m_height;
	m_currentRenderTargetView = m_gameWindow.m_frameBuffer;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 0xFF);
}

void SurfaceDirect3D11::endGameScene()
{
	scheduleBatch();
	ID3D11DeviceContext_OMSetRenderTargets(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, SDL_reinterpret_cast(ID3D11RenderTargetView**, &m_mainRenderTargetView), NULL);
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	m_currentRenderTargetView = m_mainRenderTargetView;
	updateViewport();
}

void SurfaceDirect3D11::drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	m_binded_texture = NULL;

	ID3D11ShaderResourceView* resources = NULL;
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendMod), 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);

	std::vector<Sint32> startDraws; startDraws.reserve(height + 1);
	std::vector<Sint32> countDraws; countDraws.reserve(height + 1);

	Sint32 drawY = y - scale - (scale / 2), verticeCount = 0;
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 startDraw = verticeCount;
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - (scale / 2);
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(lightmap[offset1 + offset].r, lightmap[offset1 + offset].g, lightmap[offset1 + offset].b, 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(lightmap[offset2 + offset].r, lightmap[offset2 + offset].g, lightmap[offset2 + offset].b, 255));
			verticeCount += 2;
			drawX += scale;
		}

		drawY += scale;
		startDraws.emplace_back(startDraw);
		countDraws.emplace_back(verticeCount - startDraw);
	}

	if(!updateVertexBuffer(&m_vertices[0], sizeof(VertexD3D11) * verticeCount))
		return;

	m_vertices.clear();
	ID3D11DeviceContext_IASetPrimitiveTopology(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	for(size_t i = 0, end = startDraws.size(); i < end; ++i)
		ID3D11DeviceContext_Draw(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), countDraws[i], startDraws[i]);

	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderTexture), NULL, 0);
}

void SurfaceDirect3D11::drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	m_binded_texture = NULL;

	ID3D11ShaderResourceView* resources = NULL;
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendMod), 0, 0xFFFFFFFF);
	ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
	ID3D11DeviceContext_PSSetShaderResources(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, 0, &resources);

	Sint32 halfScale = (scale / 2);
	Sint32 drawY = y - scale - halfScale;
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - halfScale;
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);

			//Get the colors of tile corners
			float topCenter[2][3], leftCenter[2][3], bottomCenter[2][3], rightCenter[2][3], center[2][3], topLeft[2][3], topRight[2][3], bottomLeft[2][3], bottomRight[2][3];
			topLeft[0][0] = lightmap[offset2 + offset].r * inv255f;
			topLeft[0][1] = lightmap[offset2 + offset].g * inv255f;
			topLeft[0][2] = lightmap[offset2 + offset].b * inv255f;
			topRight[0][0] = lightmap[offset2 + k + 1].r * inv255f;
			topRight[0][1] = lightmap[offset2 + k + 1].g * inv255f;
			topRight[0][2] = lightmap[offset2 + k + 1].b * inv255f;
			bottomLeft[0][0] = lightmap[offset1 + offset].r * inv255f;
			bottomLeft[0][1] = lightmap[offset1 + offset].g * inv255f;
			bottomLeft[0][2] = lightmap[offset1 + offset].b * inv255f;
			bottomRight[0][0] = lightmap[offset1 + k + 1].r * inv255f;
			bottomRight[0][1] = lightmap[offset1 + k + 1].g * inv255f;
			bottomRight[0][2] = lightmap[offset1 + k + 1].b * inv255f;

			//Get the colors of first triangles
			float v[3][2], p[2];
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY);
			v[1][0] = SDL_static_cast(float, drawX + scale);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topRight[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topCenter[0]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, leftCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale) - 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) - 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, center[0]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomLeft[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, rightCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomCenter[0]);

			//Get the colors of second triangles
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			v[1][0] = SDL_static_cast(float, drawX);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX + scale);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, topLeft[1]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, leftCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, bottomCenter[1]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, bottomRight[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, rightCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, topCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale) + 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) + 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, center[1]);

			//Use brighter color from triangles for our squares
			#define VEC_MAX(a)										\
				do {												\
					a[0][0] = UTIL_max<float>(a[0][0], a[1][0]);	\
					a[0][1] = UTIL_max<float>(a[0][1], a[1][1]);	\
					a[0][2] = UTIL_max<float>(a[0][2], a[1][2]);	\
				} while(0)

			VEC_MAX(topLeft);
			VEC_MAX(topRight);
			VEC_MAX(bottomLeft);
			VEC_MAX(bottomRight);
			VEC_MAX(leftCenter);
			VEC_MAX(bottomCenter);
			VEC_MAX(rightCenter);
			VEC_MAX(topCenter);
			VEC_MAX(center);
			#undef VEC_MAX

			//Draw Top-Left square
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topLeft[0][0] * 255.f), SDL_static_cast(Uint8, topLeft[0][1] * 255.f), SDL_static_cast(Uint8, topLeft[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255));
			
			//Draw Bottom-Left square
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomLeft[0][0] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][1] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255));
			
			//Draw Top-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topRight[0][0] * 255.f), SDL_static_cast(Uint8, topRight[0][1] * 255.f), SDL_static_cast(Uint8, topRight[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255));
			
			//Draw Bottom-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomRight[0][0] * 255.f), SDL_static_cast(Uint8, bottomRight[0][1] * 255.f), SDL_static_cast(Uint8, bottomRight[0][2] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255));
			
			m_cachedVertices += 24;
			drawX += scale;
		}

		drawY += scale;
	}

	scheduleBatch();
	ID3D11DeviceContext_OMSetBlendState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11BlendState*, m_blendBlend), 0, 0xFFFFFFFF);
}

void SurfaceDirect3D11::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	scheduleBatch();

	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if(!m_currentRenderTargetView || m_currentRenderTargetView == m_mainRenderTargetView)
		rotation = D3D11_GetCurrentRotation();

	D3D11_RECT scissorRect;
	switch(rotation)
	{
		case DXGI_MODE_ROTATION_ROTATE90:
		{
			scissorRect.left = m_viewPortH - y - h;
			scissorRect.right = m_viewPortH - y;
			scissorRect.top = x;
			scissorRect.bottom = x + h;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE180:
		{
			scissorRect.left = m_viewPortW - x - w;
			scissorRect.right = m_viewPortW - x;
			scissorRect.top = m_viewPortH - y - h;
			scissorRect.bottom = m_viewPortH - y;
		}
		break;
		case DXGI_MODE_ROTATION_ROTATE270:
		{
			scissorRect.left = y;
			scissorRect.right = y + h;
			scissorRect.top = m_viewPortW - x - w;
			scissorRect.bottom = m_viewPortW - x;
		}
		break;
		default:
		{
			scissorRect.left = m_viewPortX + x;
			scissorRect.right = m_viewPortX + x + w;
			scissorRect.top = m_viewPortY + y;
			scissorRect.bottom = m_viewPortY + y + h;
		}
		break;
	}
	ID3D11DeviceContext_RSSetScissorRects(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 1, &scissorRect);
	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_clippedRasterizer));
}

void SurfaceDirect3D11::disableClipRect()
{
	scheduleBatch();
	ID3D11DeviceContext_RSSetScissorRects(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), 0, NULL);
	ID3D11DeviceContext_RSSetState(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11RasterizerState*, m_mainRasterizer));
}

void SurfaceDirect3D11::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
		m_binded_texture = NULL;
	}

	//First Quad
	float minx0 = SDL_static_cast(float, x);
	float maxx0 = SDL_static_cast(float, x + lineWidth);
	float miny0 = SDL_static_cast(float, y);
	float maxy0 = SDL_static_cast(float, y + h);

	//Second Quad
	float minx1 = SDL_static_cast(float, x + w - lineWidth);
	float maxx1 = minx1 + SDL_static_cast(float, lineWidth);
	float miny1 = SDL_static_cast(float, y);
	float maxy1 = SDL_static_cast(float, y + h);

	//Third Quad
	float minx2 = SDL_static_cast(float, x + lineWidth);
	float maxx2 = minx2 + SDL_static_cast(float, w - (lineWidth << 1));
	float miny2 = SDL_static_cast(float, y);
	float maxy2 = SDL_static_cast(float, y + lineWidth);

	//Fourth Quad
	float minx3 = SDL_static_cast(float, x + lineWidth);
	float maxx3 = minx3 + SDL_static_cast(float, w - (lineWidth << 1));
	float miny3 = SDL_static_cast(float, y + h - lineWidth);
	float maxy3 = miny3 + SDL_static_cast(float, lineWidth);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vertices.emplace_back(minx0, miny0, texColor);
	m_vertices.emplace_back(minx0, maxy0, texColor);
	m_vertices.emplace_back(maxx0, miny0, texColor);
	m_vertices.emplace_back(maxx0, maxy0, texColor);
	m_vertices.emplace_back(minx1, miny1, texColor);
	m_vertices.emplace_back(minx1, maxy1, texColor);
	m_vertices.emplace_back(maxx1, miny1, texColor);
	m_vertices.emplace_back(maxx1, maxy1, texColor);
	m_vertices.emplace_back(minx2, miny2, texColor);
	m_vertices.emplace_back(minx2, maxy2, texColor);
	m_vertices.emplace_back(maxx2, miny2, texColor);
	m_vertices.emplace_back(maxx2, maxy2, texColor);
	m_vertices.emplace_back(minx3, miny3, texColor);
	m_vertices.emplace_back(minx3, maxy3, texColor);
	m_vertices.emplace_back(maxx3, miny3, texColor);
	m_vertices.emplace_back(maxx3, maxy3, texColor);
	m_cachedVertices += 24;
}

void SurfaceDirect3D11::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
		ID3D11DeviceContext_PSSetShader(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11PixelShader*, m_pixelShaderSolid), NULL, 0);
		m_binded_texture = NULL;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vertices.emplace_back(minx, miny, texColor);
	m_vertices.emplace_back(minx, maxy, texColor);
	m_vertices.emplace_back(maxx, miny, texColor);
	m_vertices.emplace_back(maxx, maxy, texColor);
	m_cachedVertices += 6;
}

Direct3D11Texture* SurfaceDirect3D11::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	Direct3D11Texture* s = &m_pictures[pictureId];
	if(!createDirect3DTexture(*s, width, height, linear))
	{
		SDL_free(pixels);
		return NULL;
	}

	updateTextureData(*s, pixels);
	SDL_free(pixels);
	return s;
}

void SurfaceDirect3D11::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256])
{
	Direct3D11Texture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}
	
	float vertices[8];
	float texcoords[8];
	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, 255);
	tex = getTextureIndex(tex);

	Sint32 rx = x, ry = y;
	Uint8 character;
	for(size_t i = pos; i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\r'://carriage return
				rx = x;
				break;
			case '\n'://new line
				rx = x; ry += cH[0];
				break;
			case 0x20://space - don't draw
				rx += cW[character] + cX[0];
				break;
			case 0x0E://Special case - change rendering color
			{
				if(i + 4 < len)//First check if we have the color bytes
				{
					Uint8 red = SDL_static_cast(Uint8, text[i + 1]);
					Uint8 green = SDL_static_cast(Uint8, text[i + 2]);
					Uint8 blue = SDL_static_cast(Uint8, text[i + 3]);
					texColor = MAKE_RGBA_COLOR(red, green, blue, 255);
					i += 3;
				}
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				texColor = MAKE_RGBA_COLOR(r, g, b, 255);
				break;
			default:
			{
				float minx = SDL_static_cast(float, rx);
				float maxx = SDL_static_cast(float, rx + cW[character]);
				float miny = SDL_static_cast(float, ry);
				float maxy = SDL_static_cast(float, ry + cH[character]);

				float minu = cX[character] * tex->m_scaleW;
				float maxu = (cX[character] + cW[character]) * tex->m_scaleW;
				float minv = cY[character] * tex->m_scaleH;
				float maxv = (cY[character] + cH[character]) * tex->m_scaleH;

				vertices[0] = minx; vertices[1] = miny;
				vertices[2] = minx; vertices[3] = maxy;
				vertices[4] = maxx; vertices[5] = miny;
				vertices[6] = maxx; vertices[7] = maxy;

				texcoords[0] = minu; texcoords[1] = minv;
				texcoords[2] = minu; texcoords[3] = maxv;
				texcoords[4] = maxu; texcoords[5] = minv;
				texcoords[6] = maxu; texcoords[7] = maxv;

				drawQuad(tex, vertices, texcoords, texColor);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
}

void SurfaceDirect3D11::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

void SurfaceDirect3D11::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	tex = getTextureIndex(tex);

	Sint32 curW, curH, cx;
	for(Sint32 j = h; j > 0; j -= sh)
	{
		curH = (j > sh ? sh : j);
		cx = x;
		for(Sint32 k = w; k > 0; k -= sw)
		{
			curW = (k > sw ? sw : k);
			float minx = SDL_static_cast(float, cx);
			float maxx = SDL_static_cast(float, cx + curW);
			float miny = SDL_static_cast(float, y);
			float maxy = SDL_static_cast(float, y + curH);

			float minu = sx * tex->m_scaleW;
			float maxu = (sx + curW) * tex->m_scaleW;
			float minv = sy * tex->m_scaleH;
			float maxv = (sy + curH) * tex->m_scaleH;

			vertices[0] = minx; vertices[1] = miny;
			vertices[2] = minx; vertices[3] = maxy;
			vertices[4] = maxx; vertices[5] = miny;
			vertices[6] = maxx; vertices[7] = maxy;

			texcoords[0] = minu; texcoords[1] = minv;
			texcoords[2] = minu; texcoords[3] = maxv;
			texcoords[4] = maxu; texcoords[5] = minv;
			texcoords[6] = maxu; texcoords[7] = maxv;

			drawQuad(tex, vertices, texcoords);
			cx += sw;
		}
		y += sh;
	}
}

void SurfaceDirect3D11::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Direct3D11Texture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + w) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + h) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

bool SurfaceDirect3D11::loadSprite(Uint32 spriteId, Direct3D11Texture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(SDL_reinterpret_cast(ID3D11Texture2D*, texture->m_texture), &stagingTextureDesc);
	stagingTextureDesc.Width = 32;
	stagingTextureDesc.Height = 32;
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
	{
		SDL_free(pixels);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_WRITE, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, textureMemory.pData);
	UINT length = 32 * 4;
	if(length == textureMemory.RowPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, pixels), length * 32);
	else
	{
		unsigned char* srcPixels = pixels;
		if(length > textureMemory.RowPitch)
			length = textureMemory.RowPitch;
		for(Uint32 row = 0; row < 32; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, srcPixels), length);
			srcPixels += length;
			dstData += textureMemory.RowPitch;
		}
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), 0, xoff, yoff, 0, SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, NULL);
	SAFE_RELEASE(stagingTexture);
	SDL_free(pixels);
	return true;
}

bool SurfaceDirect3D11::loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, Direct3D11Texture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return false;

	ID3D11Texture2D* stagingTexture;
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	ID3D11Texture2D_GetDesc(SDL_reinterpret_cast(ID3D11Texture2D*, texture->m_texture), &stagingTextureDesc);
	stagingTextureDesc.Width = 32;
	stagingTextureDesc.Height = 32;
	stagingTextureDesc.BindFlags = 0;
	stagingTextureDesc.MiscFlags = 0;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	HRESULT result = ID3D11Device_CreateTexture2D(SDL_reinterpret_cast(ID3D11Device*, m_device), &stagingTextureDesc, NULL, SDL_reinterpret_cast(ID3D11Texture2D**, &stagingTexture));
	if(FAILED(result))
	{
		SDL_free(pixels);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE textureMemory;
	result = ID3D11DeviceContext_Map(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, D3D11_MAP_WRITE, 0, &textureMemory);
	if(FAILED(result))
	{
		SAFE_RELEASE(stagingTexture);
		SDL_free(pixels);
		return false;
	}

	unsigned char* dstData = SDL_reinterpret_cast(unsigned char*, textureMemory.pData);
	UINT length = 32 * 4;
	if(length == textureMemory.RowPitch)
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, pixels), length * 32);
	else
	{
		unsigned char* srcPixels = pixels;
		if(length > textureMemory.RowPitch)
			length = textureMemory.RowPitch;
		for(Uint32 row = 0; row < 32; ++row)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dstData), SDL_reinterpret_cast(const Uint8*, srcPixels), length);
			srcPixels += length;
			dstData += textureMemory.RowPitch;
		}
	}
	ID3D11DeviceContext_Unmap(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0);
	ID3D11DeviceContext_CopySubresourceRegion(SDL_reinterpret_cast(ID3D11DeviceContext*, m_context), SDL_reinterpret_cast(ID3D11Resource*, texture->m_texture), 0, xoff, yoff, 0, SDL_reinterpret_cast(ID3D11Resource*, stagingTexture), 0, NULL);
	SAFE_RELEASE(stagingTexture);
	SDL_free(pixels);
	return true;
}

void SurfaceDirect3D11::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	drawSprite(spriteId, x, y, 32, 32, 0, 0, 32, 32);
}

void SurfaceDirect3D11::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = 0;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, spriteId);
	#endif
	Uint32 xOffset;
	Uint32 yOffset;
	Direct3D11Texture* tex;
	U64BD3D11Textures::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		if(m_sprites.size() >= MAX_SPRITES)
		{
			while(++m_spriteChecker < MAX_SPRITES)
			{
				Uint64 oldSpriteId = m_spritesIds.front();
				it = m_sprites.find(oldSpriteId);
				if(it == m_sprites.end())
				{
					UTIL_MessageBox(true, "Direct3D11: Sprite Manager failure.");
					exit(-1);
				}

				m_spritesIds.pop_front();
				if(it->second.m_lastUsage == m_currentFrame)
					m_spritesIds.push_back(oldSpriteId);
				else
					goto Skip_Search;
			}
			scheduleBatch();

			it = m_sprites.find(m_spritesIds.front());
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Direct3D11: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			Direct3D11SpriteData sprData = it->second;
			sprData.m_lastUsage = m_currentFrame;
			m_sprites.erase(it);
			m_sprites[tempPos] = sprData;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		else
		{
			Uint32 spriteIndex = SDL_static_cast(Uint32, m_sprites.size());
			Uint32 spriteAtlas = spriteIndex / m_spritesPerAtlas;
			spriteIndex = (spriteIndex % m_spritesPerAtlas) * 32;
			if(spriteAtlas >= m_spritesAtlas.size())
				return;

			Direct3D11SpriteData& sprData = m_sprites[tempPos];
			sprData.m_xOffset = spriteIndex % m_spritesPerModulo;
			sprData.m_yOffset = (spriteIndex / m_spritesPerModulo) * 32;
			sprData.m_surface = spriteAtlas;
			sprData.m_lastUsage = m_currentFrame;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
	}
	else
	{
		xOffset = it->second.m_xOffset;
		yOffset = it->second.m_yOffset;
		tex = &m_spritesAtlas[it->second.m_surface];
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

void SurfaceDirect3D11::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	drawSpriteMask(spriteId, maskSpriteId, x, y, 32, 32, 0, 0, 32, 32, outfitColor);
}

void SurfaceDirect3D11::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
{
	if(spriteId > g_spriteCounts || maskSpriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = outfitColor;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, spriteId) | (SDL_static_cast(Uint64, outfitColor) << 32);
	#endif
	Uint32 xOffset;
	Uint32 yOffset;
	Direct3D11Texture* tex;
	U64BD3D11Textures::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		if(m_sprites.size() >= MAX_SPRITES)
		{
			while(++m_spriteChecker < MAX_SPRITES)
			{
				Uint64 oldSpriteId = m_spritesIds.front();
				it = m_sprites.find(oldSpriteId);
				if(it == m_sprites.end())
				{
					UTIL_MessageBox(true, "Direct3D11: Sprite Manager failure.");
					exit(-1);
				}

				m_spritesIds.pop_front();
				if(it->second.m_lastUsage == m_currentFrame)
					m_spritesIds.push_back(oldSpriteId);
				else
					goto Skip_Search;
			}
			scheduleBatch();

			it = m_sprites.find(m_spritesIds.front());
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Direct3D11: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			Direct3D11SpriteData sprData = it->second;
			sprData.m_lastUsage = m_currentFrame;
			m_sprites.erase(it);
			m_sprites[tempPos] = sprData;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		else
		{
			Uint32 spriteIndex = SDL_static_cast(Uint32, m_sprites.size());
			Uint32 spriteAtlas = spriteIndex / m_spritesPerAtlas;
			spriteIndex = (spriteIndex % m_spritesPerAtlas) * 32;
			if(spriteAtlas >= m_spritesAtlas.size())
				return;

			Direct3D11SpriteData& sprData = m_sprites[tempPos];
			sprData.m_xOffset = spriteIndex % m_spritesPerModulo;
			sprData.m_yOffset = (spriteIndex / m_spritesPerModulo) * 32;
			sprData.m_surface = spriteAtlas;
			sprData.m_lastUsage = m_currentFrame;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		if(!loadSpriteMask(spriteId, maskSpriteId, outfitColor, tex, xOffset, yOffset))
			return;//load failed
	}
	else
	{
		xOffset = it->second.m_xOffset;
		yOffset = it->second.m_yOffset;
		tex = &m_spritesAtlas[it->second.m_surface];
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

Direct3D11Texture* SurfaceDirect3D11::createAutomapTile(Uint32 currentArea)
{
	std::unique_ptr<Direct3D11Texture> s;
	if(m_automapTiles.size() >= MAX_AUTOMAPTILES)
	{
		U32BD3D11Textures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())
		{
			UTIL_MessageBox(true, "Direct3D11: Sprite Manager failure.");
			exit(-1);
		}

		s = std::make_unique<Direct3D11Texture>(std::move(it->second));
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	if(!s)
	{
		s = std::make_unique<Direct3D11Texture>();
		if(!createDirect3DTexture(*s, 256, 256, false))
			return NULL;
	}
	else
		scheduleBatch();
	
	m_automapTiles[currentArea] = std::move(*s.get());
	m_automapTilesBuff.push_back(currentArea);
	return &m_automapTiles[currentArea];
}

void SurfaceDirect3D11::uploadAutomapTile(Direct3D11Texture* texture, Uint8 color[256][256])
{
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(256 * 256 * 4));
	if(!pixels)
		return;

	unsigned char* ptrpixels = pixels;
	for(Sint32 y = 0; y < 256; ++y)
	{
		for(Sint32 x = 0; x < 256; ++x)
		{
			Uint8 r, g, b;
			Uint8 c = color[x][y];
			colorFrom8bit(c, r, g, b);
			*ptrpixels++ = b;
			*ptrpixels++ = g;
			*ptrpixels++ = r;
			*ptrpixels++ = 255;
		}
	}
	updateTextureData(*texture, pixels);
	SDL_free(pixels);
}

void SurfaceDirect3D11::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	Direct3D11Texture* tex;
	U32BD3D11Textures::iterator it = m_automapTiles.find(currentArea);
	if(it == m_automapTiles.end())
	{
		tex = createAutomapTile(currentArea);
		if(!tex)
			return;//create failed

		uploadAutomapTile(tex, color);
		recreate = false;
	}
	else
	{
		tex = &it->second;
		if(m_automapTilesBuff.front() == currentArea)
		{
			m_automapTilesBuff.pop_front();
			m_automapTilesBuff.push_back(currentArea);
		}
		if(recreate)
		{
			uploadAutomapTile(tex, color);
			recreate = false;
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}
#endif
