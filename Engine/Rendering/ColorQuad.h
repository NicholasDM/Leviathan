#ifndef LEVIATHAN_RENDERING_QUAD
#define LEVIATHAN_RENDERING_QUAD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ResolutionScaling.h"
#include "RenderSavedResource.h"
#include <d3dx10math.h>

#include "BaseModel.h"
#include "FileSystem.h"
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif

#define COLOR_QUAD_COLOR_STYLE_LEFT_TOP_RIGHT_BOTTOM	1
#define COLOR_QUAD_COLOR_STYLE_RIGHT_BOTTOM_LEFT_TOP	2

namespace Leviathan{
	class ColorQuad : public SavedResource{
	public:
		DLLEXPORT ColorQuad();
		DLLEXPORT ~ColorQuad();
	
		//DLLEXPORT void SetColors(ID3D11DeviceContext* devcont, Float4& col1, Float4& col2);
		DLLEXPORT bool Init(ID3D11Device* device,  int screenwidth, int screenheight, int quadwidth, int quadheight, int colorstyle = 1);
		DLLEXPORT void Release();

		//DLLEXPORT void Resize(int screenwidth, int screenheight, int quadwidth, int quadheight);
		DLLEXPORT void Render(ID3D11DeviceContext* devcont, int posx, int posy, int screenwidth, int screenheight, int quadwidth, int quadheight, bool absolute = false, int colorstyle = 1);

		DLLEXPORT int GetIndexCount();


	private:
		struct VertexType{
			D3DXVECTOR3 position;
			D3DXVECTOR2 colorcoordinate;
		};


		bool InitBuffers(ID3D11Device* device);

		//bool QuickBUpdate(ID3D11DeviceContext* devcont);
		bool UpdateBuffers(ID3D11DeviceContext* devcont, int posx, int posy, int screenwidth, int screenheight, int quadwidth, int quadheight, bool absolute,int colorstyle = 1);
		void RenderBuffers(ID3D11DeviceContext* devcont);

		bool LoadTextures(ID3D11Device* dev, wstring file, wstring file2);
		// ---------------------- //
		bool Inited;




		ID3D11Buffer* Vertexbuffer;
		ID3D11Buffer* Indexbuffer;
		int VertexCount, IndexCount;
		//TextureArray* pTexture;

		int Colorstyle;
		int ScreenWidth, ScreenHeight;
		int BitmapWidth, BitmapHeight;
		int PreviousPosX, PreviousPosY;

		//Float4 start;
		//Float4 end;


	};
}
#endif