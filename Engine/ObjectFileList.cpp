#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_LIST
#include "ObjectFileList.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileList::ObjectFileList() : Lines(){
	Variables = new NamedVars();
}
ObjectFileList::ObjectFileList(const wstring &name) : Lines(){
	Name = name;
	Variables = new NamedVars();
}
ObjectFileList::~ObjectFileList(){
	SAFE_DELETE(Variables);

	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //
ScriptList* ObjectFileList::AllocateNewListFromData(){
	unique_ptr<ScriptList> obj(new ScriptList(Name));

	obj->Variables = new NamedVars(*this->Variables);
	for(unsigned int i = 0; i < Lines.size(); i++){
		obj->Lines.push_back(new wstring(*Lines[i]));
	}

	// returning smart pointer //
	ScriptList* tempptr = obj.get();
	// don't accidently delete here (don't use .reset()) //
	obj.release();
	return tempptr;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //