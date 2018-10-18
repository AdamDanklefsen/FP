/******************************************************************************************
*	Chili DirectX Framework Version 16.07.20											  *
*	Graphics.h																			  *
*	Copyright 2016 PlanetChili <http://www.planetchili.net>								  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#pragma once
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "D2d1")

#include "ChiliWin.h"
#include <d3d11.h>
#include <d2d1.h>
#include <wrl.h>
#include "ChiliException.h"
#include "Colors.h"
#include <chrono>
#include "Vec2D.h"
#include <dwrite.h>
#include <dxgi.h>

//TODO add text with DX or from bmp files
class Graphics
{
public:
	class Exception : public ChiliException
	{
	public:
		Exception( HRESULT hr,const std::wstring& note,const wchar_t* file,unsigned int line );
		std::wstring GetErrorName() const;
		std::wstring GetErrorDescription() const;
		virtual std::wstring GetFullMessage() const override;
		virtual std::wstring GetExceptionType() const override;
	private:
		HRESULT hr;
	};
private:
	// vertex format for the framebuffer fullscreen textured quad
	struct FSQVertex
	{
		float x,y,z;		// position
		float u,v;			// texcoords
	};
public:
	Graphics(class HWNDKey& key); HWND hwnd;
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
	void EndFrame();
	void BeginFrame(bool clearback);
	void setWinPos(HWND hwnd, int x, int y);
	bool onScreen(int x, int y);
	void PutPixel( int x,int y,int r,int g,int b, bool backg) { PutPixel( x,y,{ unsigned char( r ),unsigned char( g ),unsigned char( b ) }, backg); }
	void PutPixel( int x,int y,Color c, bool backg);
	void PutPixel(double x, double y, Color c, bool backg);
	void DrawCircle(int x, int y, int r, Color c, bool backg);
	void DrawLine(Vec2D a, Vec2D b, Color c, bool backg);
	void DrawLine(Vec2D a, Vec2D b, Color c, int t, bool backg);
	void DrawHLine(int a, int b, int y, Color c, bool backg);
	void DrawVLine(int a, int b, int x, Color c, bool backg);
	void DrawBlank(Vec2D a, Vec2D b, bool backg);
	float ftime;
	std::chrono::steady_clock::time_point Start;
	void fStart();
	float fEnd();
	~Graphics();

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain>				pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device>				pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>			pImmediateContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				pSysBufferTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	pSysBufferTextureView;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			pSamplerState;
	D3D11_MAPPED_SUBRESOURCE							mappedSysBufferTexture;
	Color*                                              pSysBuffer = nullptr;//Draw dynamic
public:
	Color*												BackGround = nullptr;//Draw Static
	//dynamic=Static at start -> draw dynmaic to dynmaic -> Draw
public:
	static constexpr int ScreenWidth = 1664;
	static constexpr int ScreenHeight = 936;
	static constexpr int borderx = 75, bordery = 50;
};