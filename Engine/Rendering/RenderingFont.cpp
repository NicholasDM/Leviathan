#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FONT
#include "RenderingFont.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DDsHandler.h"

#include "FileSystem.h"
#include "..\GuiPositionable.h"
#include "..\DataStore.h"
#include "Graphics.h"
#include "..\ScaleableFreeTypeBitmap.h"
#include "..\ExceptionInvalidType.h"

RenderingFont::RenderingFont(){
	// set initial data to NULL //
	Textures = NULL;
	FontsFace = NULL;
}
RenderingFont::~RenderingFont(){

}

bool RenderingFont::FreeTypeLoaded = false;
FT_Library RenderingFont::FreeTypeLibrary = FT_Library();
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::Init(ID3D11Device* dev, const wstring &FontFile){
	// get name from the filename //
	Name = FileSystem::RemoveExtension(FontFile, true);

	// load texture //
	if(!LoadTexture(dev, FontFile)){
		Logger::Get()->Error(L"RenderingFont: Init: texture loading failed, file: "+FontFile);
		return false;
	}

	// load character data //
	if(!LoadFontData(dev, FontFile)){
		Logger::Get()->Error(L"RenderingFont: Init: could not load font data, file: "+FontFile, true);
		return false;
	}
	// succeeded //
	return true;
}
void RenderingFont::Release(){
	// release FreeType objects //
	if(FontsFace)
		FT_Done_Face(FontsFace);

	SAFE_RELEASEDEL(Textures);
	SAFE_DELETE_VECTOR(FontData);
}
// ------------------------------------ //
DLLEXPORT float Leviathan::RenderingFont::CountLength(const wstring &sentence, float heightmod, int Coordtype){
	// call another function, this is so that the parameters don't all need to be created to call it //
	float delimiter = 0;
	size_t lastfit = 0;
	float fitlength = 0;
	// actual counting //
	return CalculateTextLengthAndLastFitting(heightmod, Coordtype, sentence, fitlength, lastfit, delimiter);
}
DLLEXPORT float Leviathan::RenderingFont::GetHeight(float heightmod, int Coordtype){
	if(Coordtype != GUI_POSITIONABLE_COORDTYPE_RELATIVE)
		return FontHeight*heightmod;
	// if it is relative, scale height by window size // 
	heightmod = ResolutionScaling::ScaleTextSize(heightmod);

	// scale from screen height to promilles //
	return (FontHeight*heightmod)/DataStore::Get()->GetHeight();
 }
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadFontData(ID3D11Device* dev,const wstring &file){
	// is data already in memory? //
	if(FontData.size() > 0){
		// it is //
		return true;
	}

	// check is there font data //
	wstring texturedatafile = FileSystem::ChangeExtension(file, L"levfd");
	if(!FileSystem::FileExists(texturedatafile)){
		Logger::Get()->Info(L"Font data file doesn't exist, creating..., regenerating texture", true);
		// create data since it doesn't exist //
		if(!LoadTexture(dev, file, true)){
			Logger::Get()->Error(L"LoadFontData: no data file found and generating it from font file failed", true);
			return false;
		}
		// all data should now be properly loaded //
		return true;
	}
	// load data from file //
	wifstream reader;
	reader.open(texturedatafile);
	if(!reader.is_open())
		return false;

	// read in height //
	reader >> FontHeight;
	// character count //
	int DataToRead = 0;
	reader >> DataToRead;
	// resize to have enough space at once //
	FontData.resize(DataToRead, NULL);

	for(int i = 0; (i < DataToRead) && reader.good(); i++){
		// create object for loading //
		int CCode = 0;
		UINT GlyphIndex = 0;

		// read data //
		reader >> CCode;
		reader >> GlyphIndex;

		if(CCode == 0){
			// invalid line //
			Logger::Get()->Error(L"RenderingFont: LoadFontData: invalid levfd data, file: "+file+L"number: "+Convert::ToWstring(i+1));
			continue;
		}

		// create new instance //
		unique_ptr<FontsCharacter> Curload = unique_ptr<FontsCharacter>(new FontsCharacter(CCode, GlyphIndex));

		// load rest of data to the new instance //
		reader >> Curload->PixelWidth;
		reader >> Curload->AdvancePixels;
		// texture coordinates //
		reader >> Curload->TopLeft.X;
		reader >> Curload->TopLeft.Y;
		reader >> Curload->BottomRight.X;
		reader >> Curload->BottomRight.Y;
		// set to right spot //
		size_t spot = ConvertCharCodeToIndex(CCode);
		if(FontData[spot] != NULL){
			// that's an error //
			Logger::Get()->Error(L"RenderingFont: LoadFontData: data has multiple characters with charcode: "+Convert::ToWstring(CCode)+L", file: "
				+file);
			SAFE_DELETE(FontData[spot]);
		}
		// set //
		FontData[spot] = Curload.release();
	}
	reader.close();
	// data loaded //
	return true;
}
// ------------------------------------ //
bool Leviathan::RenderingFont::LoadTexture(ID3D11Device* dev, const wstring &file, bool forcegen /*= false*/){
	// check does file exist //
	if(forcegen || !FileSystem::FileExists(file)){
		Logger::Get()->Info(L"No font texture found: generating...", false);
		// try to generate one //
		if(!_VerifyFontFTDataLoaded()){
			Logger::Get()->Error(L"RenderingFont: LoadTexture: failed to load FreeType face, cannot generate file: "+file, true);
			return false;
		}
		// multiplier that increases generated fonts size //
		int fontsizemultiplier = DataStore::Get()->GetFontSizeMultiplier();

		FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, FONT_BASEPIXELHEIGHT*fontsizemultiplier);
		if(errorcode){
			Logger::Get()->Error(L"FontGenerator: FreeType: set pixel size failed", true);
			return false;
		}

		// height "should" be forced to be this //
		int Height = FontHeight = (FONT_BASEPIXELHEIGHT*fontsizemultiplier)+2;
		int baseline = Height-((int)floorf(Height/3.f));

		// "image" //
		ScaleableFreeTypeBitmap FinishedImage(512, 512);

		// reserve data in the font data holder //
		FontData.resize(RENDERINGFONT_CHARCOUNT, NULL);

		// shortcut to glyph //
		FT_GlyphSlot slot = FontsFace->glyph;

		// vector that marks which characters are already on a row //
		vector<bool> FittedCharacters(FontData.size(), false);

		bool done = false;
		const int RowMaxWidth = 512;
		int FillingRow = 0;
		int CurrentRowLength = 0;
		int RowX = 0;

		while(!done){

			bool Fitted = false;
			bool CheckedAny = false;

			// calculate start y for this row //
			const int RowY = FillingRow*FontHeight;

			// should go from 32 to RENDERINGFONT_MAXCHARCODE
			for(size_t i = 0; i < FontData.size(); i++){
				if(FittedCharacters[i])
					continue;

				// at least checking a character //
				CheckedAny = true;

				wchar_t chartolook = (wchar_t)(i+32);

				// check do we need to generate new object //
				if(FontData[i] == NULL){
					// create new instance, with getting index of glyph //
					unique_ptr<FontsCharacter> CurChar(new FontsCharacter(chartolook, FT_Get_Char_Index(FontsFace, chartolook)));
					// generation specific data //
					CurChar->Generating = new GeneratingDataForCharacter();

					// load glyph for metrics //
					errorcode = FT_Load_Glyph(FontsFace, CurChar->CharacterGlyphIndex, FT_LOAD_DEFAULT);
					if(errorcode){
						goto glyphprocesserrorlabel;
					}
					// copy the bitmap //
					errorcode = FT_Get_Glyph(slot, &CurChar->Generating->ThisRendered);
					if(errorcode){
						goto glyphprocesserrorlabel;
					}

					// space needs special treatment //
					if(chartolook == L' '){
						// we need to make up stuff about space character //
						CurChar->PixelWidth = 6;
						CurChar->AdvancePixels = 6;

					} else {
						// set various things //
						CurChar->PixelWidth = slot->advance.x >> 6;
						CurChar->AdvancePixels = slot->advance.x >> 6;
					}

					// set instance pointer //
					FontData[i] = CurChar.release();
				}
				// accessibility ptr //
				FontsCharacter* CurChar = FontData[i];
				// now we can try to fit this character somewhere //

				// check can it fit //
				if(CurrentRowLength+FontData[i]->PixelWidth > RowMaxWidth){
					// can not fit //
					continue;
				}
				// it can fit, render it here //

				// render bitmap //
				errorcode = FT_Glyph_To_Bitmap(&CurChar->Generating->ThisRendered, FT_RENDER_MODE_NORMAL, NULL, true);
				if(errorcode){
					goto glyphprocesserrorlabel;
				}
				// get bitmap //
				FT_Bitmap& charimg = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->bitmap;


				// important data for rendering //
				CurChar->Generating->RenderedTop = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->top;
				CurChar->Generating->RenderedLeft = ((FT_BitmapGlyph)CurChar->Generating->ThisRendered)->left;

				// calculate copy position //
				int BitmapPosX = RowX+CurChar->Generating->RenderedLeft;
				int BitmapPosY = RowY+abs(CurChar->Generating->RenderedTop-baseline);

				if(BitmapPosY < RowY){
					// something is probably wrong //
					//DEBUG_BREAK;
				}

				// copy bitmap //
				FinishedImage.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);
				//FinishedImage.OutPutToFile1And0(L"testoutput.txt");
				// set font data's texture coordinate positions //
				FontData[i]->TopLeft = Float2((float)RowX, (float)RowY);
				FontData[i]->BottomRight = Float2((float)(RowX+CurChar->AdvancePixels), (float)(RowY+FontHeight-1));

				// increment StartX according to width //
				RowX += CurChar->AdvancePixels+1;
				CurrentRowLength = RowX+1;
				// set as added //
				FittedCharacters[i] = true;
				// set that a character (at least one) fit to the current row //
				Fitted = true;

				continue;
				// fail check //
glyphprocesserrorlabel:
				DEBUG_BREAK;
				Logger::Get()->Error(L"FontGenerator: FreeType: action failed, on glyph "+Convert::ToWstring(i+32));
				// "fake" that this has been added //
				FittedCharacters[i] = true;
				continue;
			}
			// checks //
			if(!CheckedAny){
				// we are done //
				done = true;
				break;
			}
			if(!Fitted){
				// no character could fit to this row, move to next //
				FillingRow++;
				CurrentRowLength = 0;
				RowX = 0;
			}
		}
		// font data is now almost done //

		// make sure that width and height are dividable by 2 //
		FinishedImage.MakeSizeDividableBy2();

		// update the main image //
		FinishedImage.UpdateStats();
		//FinishedImage.OutPutToFile1And0(L"testoutput.txt");

		int RImgWidth = FinishedImage.GetWidth();
		int RImgHeight = FinishedImage.GetHeight();

		// now that image is actually finished we can calculate actual texture coordinates //
		for(size_t i = 0; i < FontData.size(); i++){
			// calculate final texture coordinates //
			FontData[i]->TopLeft = Float2(FontData[i]->TopLeft.X/RImgWidth, FontData[i]->TopLeft.Y/RImgHeight);
			FontData[i]->BottomRight = Float2(FontData[i]->BottomRight.X/RImgWidth, FontData[i]->BottomRight.Y/RImgHeight);

			// release data //
			FT_Done_Glyph(FontData[i]->Generating->ThisRendered);
			SAFE_DELETE(FontData[i]->Generating);
		}
		// save FontData now that it has everything filled out //
		WriteDataToFile();

		// master image is now done //
		size_t mimgbuffersize = 0;
		int junk = 0;
		// use bitmap's function to create a DDS //
		char* MainImageBuffer = FinishedImage.GenerateDDSToMemory(mimgbuffersize, junk);
		// file name should always be this //
		wstring file = FileSystem::GetFontFolder()+Name+L".dds";
		Logger::Get()->Info(L"RenderingFont: LoadTexture: writing texture file, file: "+file);

		ofstream writer;
		// remember to open in binary mode or things will break //
		writer.open(Convert::WstringToString(file), ios::binary);
		if(!writer.is_open()){
			// error //
			Logger::Get()->Error(L"RenderingFont: LoadTexture: failed to write font texture to file: "+file);

			SAFE_DELETE(MainImageBuffer);
			return false;
		}
		// save the DDS file to actual file //
		writer.write(MainImageBuffer, mimgbuffersize);
		writer.close();

		return false;

		Logger::Get()->Info(L"RenderingFont: LoadTexture: successfully generated texture file: "+file);
	}

	Textures = new TextureArray();
	if(!Textures)
		return false;

	if(!Textures->Init(dev, FileSystem::GetFontFolder()+Name+L".dds", L"")){
		Logger::Get()->Error(L"LoadTexture failed Texture init returned false", true);
		return false;
	}

	return true;
}
// ------------------------------------ //
bool Leviathan::RenderingFont::BuildVertexArray(VertexType* vertexptr, const wstring &text, float drawx, float drawy, float textmodifier, int Coordtype){
	// if it is non absolute and translate size is true, scale height by window size // 
	if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		textmodifier = ResolutionScaling::ScaleTextSize(textmodifier);
	}
	throw ExceptionInvalidType(L"commented out", 0, __WFUNCTION__, L"all", L"void");
	//int index = 0;
	//if(!FontData.size()){
	//	Logger::Get()->Error(L"Trying to render font which doesn't have Fontdata");
	//	Release();
	//	SAFE_DELETE_ARRAY(vertexptr);
	//	return false;
	//}

	//// draw letters to vertices //
	//for(size_t i = 0; i < text.size(); i++){
	//	int letterindex = ((int)text[i]) - 33; // no space character in letter data array //

	//	if(letterindex < 1){
	//		// space move pos over //
	//		drawx += 3.0f*textmodifier;

	//	} else {

	//		if(Coordtype == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

	//			float AbsolutedWidth = FontData[letterindex]->PixelWidth*textmodifier;
	//			float AbsolutedHeight = FontHeight*textmodifier;

	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 0.0f);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 1.0f);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy - AbsolutedHeight, 0.0f); // bottom left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 1.0f);
	//			index++;
	//			// second triangle //
	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 0.0);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth , drawy, 0.0f); // top right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 0.0f);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + AbsolutedWidth, drawy - AbsolutedHeight, 0.0f); // bottom right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 1.0f);
	//			index++;

	//			// update location //
	//			drawx += textmodifier*1.0f + (FontData[letterindex]->PixelWidth*textmodifier);

	//		} else {

	//			// first triangle //
	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 0.0);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex]->PixelWidth*textmodifier) , drawy - (FontHeight*textmodifier), 0.0f); // bottom right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 1.0f);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy - (FontHeight*textmodifier), 0.0f); // bottom left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 1.0f);
	//			index++;
	//			// second triangle //
	//			vertexptr[index].position = D3DXVECTOR3(drawx, drawy, 0.0f); // top left
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->TopLeft, 0.0);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex]->PixelWidth*textmodifier) , drawy, 0.0f); // top right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 0.0f);
	//			index++;

	//			vertexptr[index].position = D3DXVECTOR3(drawx + (FontData[letterindex]->PixelWidth*textmodifier), drawy - (FontHeight*textmodifier), 0.0f); // bottom right
	//			vertexptr[index].texture = D3DXVECTOR2(FontData[letterindex]->BottomRight, 1.0f);
	//			index++;

	//			// update location //
	//			drawx += textmodifier*1.0f + (FontData[letterindex]->PixelWidth*textmodifier);
	//		}
	//	}
	//}
	//return true;
}

DLLEXPORT bool Leviathan::RenderingFont::RenderSentenceToTexture(const int &TextureID, const float &sizemodifier, const wstring &text, 
	Int2 &RenderedToBox, int &baselinefromimagetop)
{
	// first we need to calculate how large the bitmap is going to be //
	// open FT data //
	if(!_VerifyFontFTDataLoaded()){
		// can't do anything //
		return false;
	}

	// set right size //
	FT_Error errorcode = FT_Set_Pixel_Sizes(FontsFace, 0, (UINT)(32*sizemodifier+0.5f));
	if(errorcode){
		Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: set pixel size failed", true);
		return false;
	}
	// create bitmap matching "size" //
	ScaleableFreeTypeBitmap bitmap((int)(text.size()*28*sizemodifier), (int)(32*sizemodifier+0.5f));

	int PenPosition = 0;

	// set base line to be 1/3 from the bottom (actually the top of the bitmap's coordinates) //
	int baseline = (int)(((32*sizemodifier)/3)-0.5f);

	bitmap.SetBaseLine((int)((32*sizemodifier)-baseline));


	FT_GlyphSlot slot = FontsFace->glyph;

	// fill it with data //
	for(size_t i = 0; i < text.size(); i++){
		if(text[i] < 32){
			// whitespace //
			PenPosition += 3;
			continue;
		}
		// load glyph //
		errorcode = FT_Load_Char(FontsFace, text[i], FT_LOAD_RENDER);
		if(errorcode){
			Logger::Get()->Error(L"RenderSentenceToTexture: FreeType: failed to load glyph "+text[i]);
			continue;
		}

		// get the bitmap //
		FT_Bitmap& charimg = slot->bitmap;

		int BitmapPosX = PenPosition+slot->bitmap_left;
		int BitmapPosY = slot->bitmap_top-baseline;

		// render the bitmap to the result bitmap //
		bitmap.RenderFTBitmapIntoThis(BitmapPosX, BitmapPosY, charimg);

		// advance position //
		PenPosition += slot->advance.x >> 6;
	}
	// determine based on parameters what to do //

	// use the method to create DDS file to memory //
	size_t MemorySize = 0;
	// copy bitmap to DDS in memory and fetch the baseline height in the image to the return value //
	char* FileInMemory = bitmap.GenerateDDSToMemory(MemorySize, baselinefromimagetop);

	// we can copy the bitmap's calculated values to the size of the box //
	bitmap.CopySizeToVal(RenderedToBox);
	ID3D11ShaderResourceView* tempview = NULL;

	HRESULT hr = D3DX11CreateShaderResourceViewFromMemory(Graphics::Get()->GetRenderer()->GetDevice(), FileInMemory, MemorySize, NULL, NULL, 
		&tempview, NULL);
	if(FAILED(hr)){

		DEBUG_BREAK;
		SAFE_DELETE(FileInMemory);
		return false;
	}
	// load texture from that file and add it to texture id //
	Graphics::Get()->GetTextureManager()->AddVolatileGenerated(TextureID, L"RenderingFont", tempview);
	// release memory //
	SAFE_DELETE(FileInMemory);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::AdjustTextSizeToFitBox(const float &Size, const Float2 &BoxToFit, const wstring &text, int CoordType, 
	size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom)
{
	// calculate theoretical max height //
	float TMax = BoxToFit.Y/GetHeight(1, CoordType);


	// make absolute modifier //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		TMax = ResolutionScaling::ScaleTextSize(TMax);
	}


	// calculate length of 3 dots //
	float dotslength = CalculateDotsSizeAtScale(TMax);

	float CalculatedTotalLength = CalculateTextLengthAndLastFitting(TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	// check did all fit //
	if(Charindexthatfits == text.size()-1){
		// everything fits at the maximum size //
		// set data and return //
		EntirelyFitModifier = HybridScale = ResolutionScaling::UnScaleTextFromSize(TMax);
		Finallength = Float2(CalculatedTotalLength, GetHeight(TMax, CoordType));

		return true;
	}

	// check at which scale the text would entirely fit //
	// AdjustedScale = original * (wanted length/got length)
	float AdjustedScale = TMax*(BoxToFit.X/CalculatedTotalLength);

	// adjusted scale is now the scale that allows all characters to fit //
	EntirelyFitModifier = ResolutionScaling::UnScaleTextFromSize(AdjustedScale);

	// check is it too low //
	if(AdjustedScale < scaletocutfrom*TMax){
		// we are going to need to count the spot where the text needs to be cut with scaletocutfrom*TMax //
		HybridScale = ResolutionScaling::UnScaleTextFromSize(scaletocutfrom*TMax);

		// new length to dots //
		dotslength = CalculateDotsSizeAtScale(scaletocutfrom*TMax);

		CalculatedTotalLength = CalculateTextLengthAndLastFitting(scaletocutfrom*TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

		Finallength = Float2(CalculatedTotalLength, GetHeight(scaletocutfrom*TMax, CoordType));
		return true;
	}

	// we can use adjusted scale to fit everything //
	HybridScale = EntirelyFitModifier;

	dotslength = CalculateDotsSizeAtScale(AdjustedScale);
	CalculatedTotalLength = CalculateTextLengthAndLastFitting(AdjustedScale, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	if(Charindexthatfits != text.size()-1){
		// something is definitely wrong //
		DEBUG_BREAK;
	}

	Finallength = Float2(CalculatedTotalLength, GetHeight(AdjustedScale, CoordType));

	return true;
}

float Leviathan::RenderingFont::CalculateTextLengthAndLastFitting(float TextSize, int CoordType, const wstring &text, const float &fitlength, 
	size_t & Charindexthatfits, float delimiterlength)
{
	// TextSize is most likely already adjusted with CoordType so don't adjust here //
	float CalculatedTotalLength = 0.f;

	bool FitsIfLastFits = false;
	float curneededlength = 0;

	// calculate length using the theoretical maximum size //
	for(size_t i = 0; i < text.size(); i++){
		// check is this whitespace //
		if(text[i] <= L' '){
			// white space //
			CalculatedTotalLength += 3.0f*TextSize;
		} else {
			// get size from letter index //
			int letterindex = ((int)text[i])-33; // no space character in letter data array //


			CalculatedTotalLength += 1.f*TextSize+FontData[letterindex]->PixelWidth*TextSize;
		}

		bool Jumped = false;

textfittingtextstartofblocklabel:


		if(FitsIfLastFits){
			// we need to only check if last character fits //
			if(i+1 < text.size()){
				// not last yet //
				continue;
			}

			// will just need to try to fit this last character //
			curneededlength = CalculatedTotalLength;
			// fall through to continue checks //

		} else {
			// check does this character and dots fit to the "box" //
			curneededlength = (CalculatedTotalLength+delimiterlength);
		}

		if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

			curneededlength /= DataStore::Get()->GetWidth();
		}

		// would all this stuff fit //
		if(curneededlength <= fitlength){
			// check are we trying to fit last character //
			if(FitsIfLastFits){
				// last character was able to fit without delimiting characters //
				Charindexthatfits = i;

			} else {
				// this character would fit with truncation to the box //
				Charindexthatfits = i;
			}
		} else {
			// this character wouldn't fit if it had to be cut from here //
			FitsIfLastFits = true;

			// check is this last character, because then we need to go back and check without delimiter //
			if(i+1 >= text.size() && !Jumped){
				// set jumped so that we can't get stuck in an infinite loop //
				Jumped = true;
				goto textfittingtextstartofblocklabel;
			}
		}
	}

	// update total length //
	if(CoordType == GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		CalculatedTotalLength /= DataStore::Get()->GetWidth();
	}

	return CalculatedTotalLength;
}

float Leviathan::RenderingFont::CalculateDotsSizeAtScale(const float &scale){
	return 3.f*(FontData[L'.'-32]->AdvancePixels*scale+1.f*scale);
}
// ------------------------------------ //
bool Leviathan::RenderingFont::_VerifyFontFTDataLoaded(){
	// verify FreeType 2 //
	if(!CheckFreeTypeLoad()){
		// can't do anything //
		return false;
	}

	// check is it already loaded //
	if(FontsFace != NULL)
		return true;

	// load file matching this font //
	wstring fontgenfile = FileSystem::SearchForFile(FILEGROUP_OTHER, Name, L"ttf", true);

	// look for it in registry //
	if(fontgenfile.size() == 0){
		// set name to arial because we can't find other fonts //
		Name = L"Arial";

		if(Name == L"Arial"){
			HKEY hKey;
			LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ,
				&hKey);

			if(lRes != ERROR_SUCCESS){
				Logger::Get()->Error(L"FontGenerator: could not locate Arial font, OpenKey failed", true);
				return false;
			}

			WCHAR szBuffer[512];
			DWORD dwBufferSize;

			RegQueryValueEx(hKey, L"Arial (TrueType)", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);

			//fontgenfile = L"C:\\Windows\\Fonts\\";
			FileSystem::GetWindowsFolder(fontgenfile);
			fontgenfile += L"Fonts\\";
			fontgenfile += szBuffer;
		}
	}
	if(!FileSystem::FileExists(fontgenfile)){
		Logger::Get()->Error(L"LoadTexture failed: could not generate new texture: no text definition file", true);
		return false;
	}

	FT_Error errorcode = FT_New_Face(this->FreeTypeLibrary, Convert::WstringToString(fontgenfile).c_str(), 0, &FontsFace);

	if(errorcode == FT_Err_Unknown_File_Format ){
		Logger::Get()->Error(L"FontGenerator: FreeType: unknown format!", true);
		return false;
	} else if (errorcode) {

		Logger::Get()->Error(L"FontGenerator: FreeType: cannot open file!", true);
		return false;
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RenderingFont::WriteDataToFile(){
	// write font data file //
	wstring datafilepath = FileSystem::GetFontFolder()+Name+L".levfd";

	// generate file content //
	wstringstream datafile;
	// character data count //
	datafile << FontHeight << L" " <<  FontData.size() << endl;

	// copy font data //
	for(size_t i = 0; i < FontData.size(); i++){
		
		// output data to stream //
		datafile << FontData[i]->CharCode << L" ";

		datafile << FontData[i]->CharacterGlyphIndex << " ";
		datafile << FontData[i]->PixelWidth << " ";
		datafile << FontData[i]->AdvancePixels << " ";
		datafile << FontData[i]->TopLeft.X << " " << FontData[i]->TopLeft.Y << " ";
		datafile << FontData[i]->BottomRight.X << " " << FontData[i]->BottomRight.Y << " ";
		// this object is written, put empty line //
		datafile << endl;
	}
	// write to file //
	wofstream writer;

	writer.open(datafilepath);
	// check can we write stuff //
	if(!writer.is_open()){
		Logger::Get()->Error(L"RenderingFont: WriteDataToFile: failed to open file for reading, file: "+datafilepath, true);
		return false;
	}

	// write the whole stringstream to the file and close //
	writer << datafile.str();
	writer.close();


	Logger::Get()->Info(L"RenderingFont: WriteDataToFile: wrote font data file, font: "+Name+L", file: "+datafilepath);
	return true;
}
// ------------------------------------ //
ID3D11ShaderResourceView* Leviathan::RenderingFont::GetTexture(){
	return Textures->GetTexture();
}

bool Leviathan::RenderingFont::CheckFreeTypeLoad(){
	if(!FreeTypeLoaded){

		FT_Error error = FT_Init_FreeType(&FreeTypeLibrary);
		if(error){

			Logger::Get()->Error(L"RenderingFont: FreeTypeLoad failed", error, true);
			return false;
		}
		FreeTypeLoaded = true;
	}
	return true;
}


// ------------------ FontsCharacter ------------------ //
Leviathan::FontsCharacter::FontsCharacter(const int &charcode, const FT_UInt &glyphindex /*= 0*/) : TopLeft(0, 0), BottomRight(0, 0), PixelWidth(0),
	Generating(NULL)
{
	CharCode = charcode;
	CharacterGlyphIndex = glyphindex;
}

Leviathan::FontsCharacter::FontsCharacter(const int &charcode, const int &pixelwidth, const Float2 &texturecoordtopleft, const Float2 
	&texturecoordbotomright, const FT_UInt &glyphindex /*= 0*/) : TopLeft(texturecoordtopleft), BottomRight(texturecoordbotomright), 
	PixelWidth(pixelwidth)
{
	CharCode = charcode;
	CharacterGlyphIndex = glyphindex;
}

Leviathan::GeneratingDataForCharacter::GeneratingDataForCharacter() : ThisRendered(NULL), RenderedTop(-1), RenderedLeft(-1){

}
