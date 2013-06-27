#ifndef LEVIATHAN_WSTRINGITERATOR
#define LEVIATHAN_WSTRINGITERATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MultiFlag.h"
#include "ExceptionNULLPtr.h"
#include "IteratorData.h"

namespace Leviathan{

	// forward declaration //
	class WstringIterator;

	enum QUOTETYPE {QUOTETYPE_DOUBLEQUOTES, QUOTETYPE_SINGLEQUOTES, QUOTETYPE_BOTH};
	enum DECIMALSEPARATORTYPE {DECIMALSEPARATORTYPE_DOT, DECIMALSEPARATORTYPE_COMMA, DECIMALSEPARATORTYPE_BOTH, DECIMALSEPARATORTYPE_NONE};
	enum UNNORMALCHARACTER {UNNORMALCHARACTER_TYPE_NON_ASCII, UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, UNNORMALCHARACTER_TYPE_WHITESPACE_AND_CONTROL,
		UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE, UNNORMALCHARACTER_TYPE_NON_NAMEVALID_WITHWHITESPACE};
	enum EQUALITYCHARACTER {EQUALITYCHARACTER_TYPE_EQUALITY, EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE, EQUALITYCHARACTER_TYPE_ALL};
	enum ITERATORCALLBACK_RETURNTYPE {ITERATORCALLBACK_RETURNTYPE_STOP, ITERATORCALLBACK_RETURNTYPE_CONTINUE};

#define WSTRINGITERATOR_IGNORE_SPECIAL			200001
#define WSTRINGITERATOR_STOP					200002
#define WSTRINGITERATOR_INSIDE_STRING			200005
#define WSTRINGITERATOR_INSIDE_STRING_DOUBLE	200006
#define WSTRINGITERATOR_INSIDE_STRING_SINGLE	200007
#define WSTRINGITERATOR_IGNORE_SPECIAL_END		200008


	typedef ITERATORCALLBACK_RETURNTYPE (*IteratorWstrCallBack)(WstringIterator* instance, Object* IteratorData, int parameters);

	// forward declarations for iterator methods //
	ITERATORCALLBACK_RETURNTYPE FindFirstQuotedString(WstringIterator* instance, Object* IteratorData, int parameters);
	ITERATORCALLBACK_RETURNTYPE FindNextNumber(WstringIterator* instance, Object* IteratorData, int parameters);
	ITERATORCALLBACK_RETURNTYPE FindNextNormalCharacterString(WstringIterator* instance, Object* IteratorData, int parameters);
	ITERATORCALLBACK_RETURNTYPE FindUntilEquality(WstringIterator* instance, Object* IteratorData, int parameters);
	ITERATORCALLBACK_RETURNTYPE FindFromStartUntilCommentOrEnd(WstringIterator* instance, Object* IteratorData, int parameters);
	ITERATORCALLBACK_RETURNTYPE FindUntilSpecificCharacter(WstringIterator* instance, Object* IteratorData, int parameters);

	// could potentially inherit from base iterator, but not right now //
	class WstringIterator : public Object{
	public:
		// ----------------- friend functions ------------------- //
		friend ITERATORCALLBACK_RETURNTYPE FindFirstQuotedString(WstringIterator* instance, Object* IteratorData, int parameters);
		friend ITERATORCALLBACK_RETURNTYPE FindNextNumber(WstringIterator* instance, Object* IteratorData, int parameters);
		friend ITERATORCALLBACK_RETURNTYPE FindNextNormalCharacterString(WstringIterator* instance, Object* IteratorData, int parameters);
		friend ITERATORCALLBACK_RETURNTYPE FindUntilEquality(WstringIterator* instance, Object* IteratorData, int parameters);
		friend ITERATORCALLBACK_RETURNTYPE FindFromStartUntilCommentOrEnd(WstringIterator* instance, Object* IteratorData, int parameters);
		friend ITERATORCALLBACK_RETURNTYPE FindUntilSpecificCharacter(WstringIterator* instance, Object* IteratorData, int parameters);
		// ------------------------------------ //
		DLLEXPORT WstringIterator::WstringIterator(const wstring& text);
		//************************************
		// Method:    WstringIterator
		// FullName:  Leviathan::WstringIterator::WstringIterator
		// Access:    public 
		// Parameter: wstring * text
		// Parameter: bool TakesOwnership
		// Usage: Creates new instance. If TakesOwnership pointer is deleted in destructor
		//************************************
		DLLEXPORT WstringIterator::WstringIterator(wstring* text, bool TakesOwnership = false);
		DLLEXPORT virtual WstringIterator::~WstringIterator();

		DLLEXPORT unique_ptr<wstring> GetStringInQuotes(QUOTETYPE quotes, bool AllowSpecialQualifiers = true);
		DLLEXPORT unique_ptr<wstring> GetNextNumber(DECIMALSEPARATORTYPE decimal);
		DLLEXPORT unique_ptr<wstring> GetNextCharacterSequence(UNNORMALCHARACTER stopcase);
		DLLEXPORT unique_ptr<wstring> GetUntilEqualityAssignment(EQUALITYCHARACTER stopcase);
		DLLEXPORT unique_ptr<wstring> GetUntilEnd();
		DLLEXPORT unique_ptr<wstring> GetUntilNextCharacterOrNothing(wchar_t charactertolookfor);

		DLLEXPORT unsigned long GetPosition();
		DLLEXPORT void SetPosition(unsigned long pos);
		DLLEXPORT wchar_t GetCurrentCharacter();
		DLLEXPORT wchar_t GetCharacterAtPos(size_t pos);
		DLLEXPORT bool MoveToNext();

		DLLEXPORT Object* StartIterating(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters);
		

		DLLEXPORT bool IsOutOfBounds(unsigned long pos);
		DLLEXPORT bool IsOutOfBounds();
		DLLEXPORT unsigned int GetWstringLength();

		DLLEXPORT void ReInit(wstring* text, bool TakesOwnership = false);
		DLLEXPORT void ReInit(const wstring& text);

		DLLEXPORT static void StripPreceedingAndTrailingWhitespaceComments(wstring &str);
	private:
		// ------------------------------------ //
		inline ITERATORCALLBACK_RETURNTYPE HandleSpecialCharacters();
		inline ITERATORCALLBACK_RETURNTYPE CheckActiveFlags();

		int HandleCurrentIteration(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters);

		// ------------------------------------ //
		// type, specifies if should try to delete the wstring on destructor //
		// bool bitfield to save memory //
		bool HandlesDelete : 1;
		bool IsPtrUsed : 1;

		wstring* Data;
		const wstring ConstData;

		// iteration data //
		unsigned long IteratorPosition;

		// currently active flags (text block, ignore next character) //
		unique_ptr<MultiFlag> CurrentFlags;
	};

}
#endif