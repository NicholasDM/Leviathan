#include "TestAutoUpdateable.h"
#include "TestCallable.h"
#include "Utility/MD5Generator.h"
#include "ObjectFiles/LineTokenizer.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Utility/Iterators/WstringIterator.h"
#include "Utility/DebugVariableNotifier.h"
#include <boost/assign/list_of.hpp>

bool TestMiscCutWstring(const int &tests){
	bool Failed = false;
	// correct result //
	wstring teststring = L"hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! -qltest -ql-over or not-ql?";
	vector<wstring> exampleresult;
	exampleresult.reserve(5);
	exampleresult.push_back(L"hey nice string ");
	exampleresult.push_back(L"you have -qlhere or ");
	exampleresult.push_back(L"mql-yaybe");
	exampleresult.push_back(L" oh no! -qltest ");
	exampleresult.push_back(L"over or not-ql?");

	// check function state //
	vector<wstring> result;

	StringOperations::CutString(teststring, wstring(L"-ql-"), result);
	// check result //
	if(result.size() != exampleresult.size()){
		Failed = true;
	} else {
		for(unsigned int ind = 0; ind < exampleresult.size(); ind++){
			if(result[ind] != exampleresult[ind])
				TESTFAIL;
		}
	}
	if(Failed){
		Logger::Get()->Error(L"EngineTest FAILED: on Misc::CutWstring wrong result");
		return false;
	}
	// stress testing function //
	for(int i = 0; i < tests; i++){
		result.clear();

		StringOperations::CutString(teststring, wstring(L"-ql-"), result);
	}
	return Failed;
}


bool TestMiscReplace(const int &tests){
	bool Failed = false;
	// correct result //
	wstring teststring = L"hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! -qltest -ql-over or not-ql?";
	wstring correctresult = L"hey nice string hey_whatsthis?you have -qlhere or hey_whatsthis?mql-yaybehey_whatsthis? oh no! -qltest hey_whatsthis?over or not-ql?";

	// check function state //
	wstring result;

	result = StringOperations::Replace<wstring>(teststring, L"-ql-", L"hey_whatsthis?");
	// check result //
	if(result != correctresult){
		TESTFAIL;
	}
	if(Failed){
		Logger::Get()->Error(L"EngineTest FAILED: on Misc::Replace wrong result");
		return false;
	}
	// stress testing function //
	for(int i = 0; i < tests; i++){

		result = StringOperations::Replace<wstring>(teststring, L"-ql-", L"hey_whatsthis?");
	}

	return Failed;
}

bool TestMiscWstringRemovePreceedingTrailingSpaces(const int &tests){
	bool Failed = false;
	// correct result //
#define ORIGINALVALUE_FOR_TEST L"		a  asd	hey nice   ?!	 "

	wstring teststringandresult = ORIGINALVALUE_FOR_TEST;
	wstring correctresult = L"a  asd	hey nice   ?!";

	// check function state //
	StringOperations::WstringRemovePreceedingTrailingSpaces(teststringandresult);
	// check result //
	if(teststringandresult != correctresult){
		TESTFAIL;
	}
	if(Failed){
		Logger::Get()->Error(L"EngineTest FAILED: on Misc::WstringRemovePreceedingTrailingSpaces wrong result");
		return false;
	}
	// stress testing function //
	for(int i = 0; i < tests; i++){
		teststringandresult = ORIGINALVALUE_FOR_TEST;
		StringOperations::WstringRemovePreceedingTrailingSpaces(teststringandresult);
	}

	return Failed;
}

bool TestAutoUpdateableFunctions(const int& tests, Engine* engine){
	// create count amount of
	bool Failed = false;
	vector<TestAutoUpdateable*> Objects;
	Objects.reserve(tests);

	int ValueID = 25665;
	wstring SemanticName = L"test value thing";
	//wstring SemanticName = L"test ";

	for(int i = 0; i < tests; i++){
		Objects.push_back(new TestAutoUpdateable());
	}

	DataStore::Get()->AddVar(new NamedVariableList(SemanticName, new VariableBlock(new IntBlock(25000))));


	vector<VariableBlock*> listenneededindexes(1);
	listenneededindexes[0] = new VariableBlock(SemanticName);

	// register all listeners //
	for(int i = 0; i < tests; i++){
		Objects[i]->StartMonitoring(listenneededindexes);
	}

	SAFE_DELETE_VECTOR(listenneededindexes);

	// send event //
	DataStore::Get()->SetValue(SemanticName, new VariableBlock(new IntBlock(25001)));


	// check did all objects receive it //
	for(int i = 0; i < tests; i++){
		if(!Objects[i]->HasUpdated()){
			TESTFAIL;
		}
	}

	if(Failed){
		Logger::Get()->Error(L"EngineTest FAILED: Autoupdateable Functions no events received");
	}

	// release objects //
	while(Objects.size() != 0){

		SAFE_DELETE(Objects[0]);
		Objects.erase(Objects.begin());
	}
	return Failed;
}

bool TestEventsFunctions(const int& tests, Engine* engine){
	// create count amount of
	bool Failed = false;
	vector<TestCallable*> Objects;
	Objects.reserve(tests);

	for(int i = 0; i < tests; i++){
		Objects.push_back(new TestCallable());
	}

	EventHandler* events = engine->GetEventHandler();

	// register all listeners //
	for(int i = 0; i < tests; i++){
		events->RegisterForEvent(Objects[i], EVENT_TYPE_TEST);

	}
	// create event //
	Event* sentevent = new Event(EVENT_TYPE_TEST, NULL);

	// send event //
	events->CallEvent(sentevent);


	// check did all objects receive it //
	for(int i = 0; i < tests; i++){
		if(!Objects[i]->IsEvented){
			TESTFAIL;
		}
	}

	if(Failed){
		Logger::Get()->Error(L"EngineTest FAILED: onEvents Functions no events received");
	}

	// release objects //
	while(Objects.size() != 0){

		SAFE_DELETE(Objects[0]);
		Objects.erase(Objects.begin());
	}
	return Failed;
}

bool TestIDFactory(const int &tests){
	// get ids from the factory //
	int id = 0;
	int latestid = -1;
	for(int i = 0; i < tests; i++){
		id = IDFactory::GetID();
		if(id == latestid){
			// error //
			Logger::Get()->Error(L"EngineTest FAILED: TestIDFactory same id in row");
			return false;
		}
		latestid = id;
	}
	// Failed = false
	return false;
}

bool TestMultiFlag(const int &tests){
	vector<shared_ptr<Flag>> Flags;
	Flags.push_back(shared_ptr<Flag>(new Flag(2500)));
	Flags.push_back(shared_ptr<Flag>(new Flag(2501)));
	Flags.push_back(shared_ptr<Flag>(new Flag(2502)));
	Flags.push_back(shared_ptr<Flag>(new Flag(2503)));
	Flags.push_back(shared_ptr<Flag>(new Flag(2504)));

	// create multiflag //
	MultiFlag* mflag = new MultiFlag(Flags);

	// check validness //
	mflag->SetFlag(Flag(2505));
	mflag->UnsetFlag(Flag(2504));

	if((!mflag->IsSet(2500)) || (!mflag->IsSet(2501)) || (!mflag->IsSet(2502)) || (!mflag->IsSet(2503)) || (!mflag->IsSet(2505))
		|| (mflag->IsSet(2504)))
	{

		Logger::Get()->Error(L"EngineTest FAILED: TestMultiFlag flags not working properly");
		return false;
	}

	SAFE_DELETE(mflag);
	// stress testing //

	for(int i = 0; i < tests; i++){
		MultiFlag* mflag = new MultiFlag(Flags);
		mflag->SetFlag(Flag(2505));
		mflag->UnsetFlag(Flag(2504));
		SAFE_DELETE(mflag);
	}
	// failed = false
	return false;
}

bool TestNamedVars(const int &tests){
	vector<shared_ptr<NamedVariableList>> Variables;
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var1", new VariableBlock(1))));
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var2", new VariableBlock(2))));
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var3", new VariableBlock(3))));
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var4", new VariableBlock(4))));
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var5", new VariableBlock(5))));

	// create holder //
	NamedVars* holder = new NamedVars();
	bool Failed = false;

	holder->SetVec(Variables);

	// add some more values //
	holder->AddVar(L"var66", new VariableBlock(25));
	holder->Remove(holder->Find(L"var2"));


	// check //
	int checkval = -1;
	wstring emptystr = L"";

	wstring typelinething = L"this= not this";

	// creation testing //
	shared_ptr<NamedVariableList> ptry(new NamedVariableList(typelinething));

	if(!ptry->GetValueDirect()->ConvertAndAssingToVariable<wstring>(emptystr)){

		TESTFAIL;
	} else {
		if(emptystr != L"not this"){
			TESTFAIL;
		}
	}

	typelinething = L"this=2";

	ptry = shared_ptr<NamedVariableList>(new NamedVariableList(typelinething));


	if(!ptry->GetValueDirect()->ConvertAndAssingToVariable<int>(checkval)){

		TESTFAIL;
	} else {
		if(checkval != 2){
			TESTFAIL;
		}
	}

	if(!holder->GetValueAndConvertTo<int>(L"var3", checkval)){

		TESTFAIL;

	} else {
		if(checkval != 3)
			TESTFAIL;
	}

	if((holder->Find(L"var66") < 0) || (holder->Find(L"var2") > -1)){
		TESTFAIL;
	}

	// advanced variable testing //
	// get pointer to value list //
	wstring linething = L"Color = [[0.1], [4], [true], [\"lol\"]]";
	NamedVariableList advlist(linething);

	// make sure that size is right and types are correct //
	if(advlist.GetVariableCount() != 4){

		TESTFAIL;
	} else {
		// check values //

		if(advlist.GetValueDirect(0)->GetBlockConst()->Type != DATABLOCK_TYPE_FLOAT){
			TESTFAIL;
		}
		if(advlist.GetValueDirect(1)->GetBlockConst()->Type != DATABLOCK_TYPE_INT){
			TESTFAIL;
		}
		if(advlist.GetValueDirect(2)->GetBlockConst()->Type != DATABLOCK_TYPE_BOOL){
			TESTFAIL;
		}
		if(advlist.GetValueDirect(3)->GetBlockConst()->Type != DATABLOCK_TYPE_WSTRING){
			TESTFAIL;
		}
	}




	// stress testing //
	for(int i = 0; i < tests; i++){
		holder->Remove(holder->Find(L"var4"));
		holder->AddVar(L"var4", new VariableBlock(25));
	}
	// release data //
	Variables.clear();
	SAFE_DELETE(holder);

	return Failed;
}

bool TestScripting(const int &tests, Engine* engine){
	// create script //
	bool Failure = false;

	//ScriptExecutor* exec = ScriptInterface::Get()->GetExecutor();

	//// setup the script //
	//ScriptModule* mod = exec->CreateNewModule(L"TestScrpt", "ScriptGenerator").lock().get();
	//shared_ptr<ScriptScript> Scriptreal = mod->GetScriptInstance();

	//// compile it //
	//mod->GetBuilder().AddSectionFromMemory("int TestFunction(int Val1, int Val2){\n"
	//	"// do some time consuming stuff //\n"
	//	"Val1 *= Val1+Val2 % 15;\n"
	//	"Val2 /= Val2-Val1-Val2*25+2;\n"
	//	"return 42;\n"
	//	"}", "test section");
	//
	//mod->GetBuilder().BuildModule();

	//mod->SetBuildState(SCRIPTBUILDSTATE_BUILT);

	//vector<shared_ptr<NamedVariableBlock>> Params = boost::assign::list_of(new NamedVariableBlock(new IntBlock(252134), L"Val1"))
	//	(new NamedVariableBlock(new IntBlock(25552), L"Val2"));
	//// call script //

	//for(int i = 0; i < tests; i++){

	//	ScriptRunningSetup ssetup;
	//	ssetup.SetArguements(Params).SetEntrypoint("TestFunction").SetUseFullDeclaration(false);

	//	shared_ptr<VariableBlock> returned = ScriptInterface::Get()->ExecuteScript(Scriptreal.get(), &ssetup);

	//	// check did it exist //
	//	int Value = *returned;
	//	if(Value != 42){
	//		// failed //
	//		Failure = true;
	//	}
	//}

	//mod->DeleteThisModule();
	return Failure;
}

bool MD5Testing(const int &tests){
	string testvalue = "string to make into md5 has!";
	bool Failed = false;

	for(int i = 0; i < tests; i++){
		string result = MD5(testvalue).hexdigest();

		// check //
		if(result != "a59e6c8c49baf73cb6c15dbc18967812"){
			TESTFAIL;
		}
	}


	return Failed;
}

void RandomTesting(const int &tests){

	// just get bunch of random numbers //
	for(int i = 0; i < tests; i++){
		int number = Random::Get()->GetNumber();
	}
}


bool LineTokenizerTest(const int &tests){

	bool Failed = false;
	// test values //
	wstring teststr = L"This is line that needs to be tokenized [[to nice], [2], [that work], [2567]] lol";

	vector<wstring> Propersplit;
	Propersplit.push_back(L"This");
	Propersplit.push_back(L"is");
	Propersplit.push_back(L"line");
	Propersplit.push_back(L"that");
	Propersplit.push_back(L"needs");
	Propersplit.push_back(L"to");
	Propersplit.push_back(L"be");
	Propersplit.push_back(L"tokenized");
	Propersplit.push_back(L"[[to nice], [2], [that work], [2567]]");
	Propersplit.push_back(L"lol");

	// split to tokens //
	vector<wstring*> TokenSplit;
	vector<Token*> Tokens;
	vector<wstring> ValueTokens;
	Leviathan::LineTokeNizer::TokeNizeLine(teststr, TokenSplit);

	if(Propersplit.size() != TokenSplit.size()){
		TESTFAIL;
	}

	for(unsigned int i = 0; i < Propersplit.size(); i++){
		if(Propersplit[i] != *TokenSplit[i]){
			TESTFAIL;
		}
	}


	if(Failed)
		goto cleanup;

	// check succeed //
	for(unsigned int i = 0; i < Propersplit.size(); i++){
		// release old, if any
		SAFE_DELETE_VECTOR(Tokens);
		LineTokeNizer::SplitTokenToRTokens(Propersplit[i], Tokens);

		// check tokens //
		switch(i){
		case 0:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"This")
					TESTFAIL;
			}
		break;
		case 1:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"is")
					TESTFAIL;
			}
		break;
		case 2:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"line")
					TESTFAIL;
			}
		break;
		case 3:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"that")
					TESTFAIL;
			}
		break;
		case 4:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"needs")
					TESTFAIL;
			}
		break;
		case 5:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"to")
					TESTFAIL;
			}
		break;
		case 6:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"be")
					TESTFAIL;
			}
		break;
		case 7:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}
				if(Tokens[0]->GetData() != L"tokenized")
					TESTFAIL;
			}
		break;
		case 8:
			{
				if(Tokens.size() != 5){
					TESTFAIL;
					continue;
				}
				if((Tokens[1]->GetData() != L"to nice") || (Tokens[2]->GetData() != L"2") || (Tokens[3]->GetData() != L"that work")
					|| (Tokens[4]->GetData() != L"2567") || (Tokens[0]->GetSubTokenCount() != 4))
				{
					TESTFAIL;
				}
			}
		break;
		case 9:
			{
				if(Tokens.size() != 1){
					TESTFAIL;
					continue;
				}

				if(Tokens[0]->GetData() != L"lol")
					TESTFAIL;
			}
		break;

		}
	}



	//if(Failed)
	//	goto cleanup;

	// stress test //
	for(int i = 0; i < tests; i++){
		SAFE_DELETE_VECTOR(TokenSplit);
		SAFE_DELETE_VECTOR(Tokens);

		Leviathan::LineTokeNizer::TokeNizeLine(teststr, TokenSplit);
		LineTokeNizer::SplitTokenToRTokens(Propersplit[8], Tokens);
	}

cleanup:
	// release all memory //
	SAFE_DELETE_VECTOR(TokenSplit);
	SAFE_DELETE_VECTOR(Tokens);

	return Failed;
}


bool ObjectFileParserTest(const int &tests){
	bool Failed = false;

	wstring TestFile = Engine::GetEngine()->GetFileSystem()->SearchForFile(FILEGROUP_SCRIPT, L"TestObjectFile", L"levof", true);
	if(TestFile == L""){

		Logger::Get()->Error(L"EngineTest FAILED: ObjectFileParserTest file TestObjectFile.levof not found", false);
		return true;
	}
	vector<shared_ptr<NamedVariableList>> HeaderVars;
	vector<shared_ptr<ObjectFileObject>> objects = ObjectFileProcessor::ProcessObjectFile(TestFile, HeaderVars);

	// check integrity //
	if(HeaderVars.size() == 4){
		if(!HeaderVars[0]->CompareName(L"FileType")){
			TESTFAIL;
		}
		if(!HeaderVars[3]->CompareName(L"Why")){
			TESTFAIL;
		}
		wstring valuescheck = L"";

		bool ivaluecheck = false;

		if(!HeaderVars[1]->GetValueDirect()->ConvertAndAssingToVariable<wstring>(valuescheck)){

			TESTFAIL;

		} else {

			if(valuescheck != L"lol"){
				TESTFAIL;
			}
		}

		if(!HeaderVars[3]->GetValueDirect()->ConvertAndAssingToVariable<bool>(ivaluecheck)){

			TESTFAIL;

		} else {

			if(ivaluecheck != true){
				// value should be "true" (1) //
				TESTFAIL;
			}
		}

		if(objects.size() == 2){
			ObjectFileObject* obj = objects[0].get();
			// check name and prefixes //
			if(obj->Name != L"Test1"){
				TESTFAIL;
			}
			if(obj->TName != L"TestData"){
				TESTFAIL;
			}

			if(obj->Prefixes.size() == 2){
				if((*obj->Prefixes[0]) != L"nice_prefix"){
					TESTFAIL;
				}

				if((*obj->Prefixes[1]) != L"type1_special"){
					TESTFAIL;
				}

			} else {

				TESTFAIL;
			}
			// check contents just so that they seem right //
			if(obj->TextBlocks.size() != 1){
				TESTFAIL;
			}
			if(obj->Contents.size() != 1){
				TESTFAIL;
			}

			// check for proper script //
			if(obj->Script.get() != NULL){
				if(obj->Script->GetModule()->GetName() != L"TestScript"){
					TESTFAIL;
				}
			} else {

				TESTFAIL;
			}
			// second object test //
			obj = objects[1].get();

			if(obj->Name != L"Something"){
				TESTFAIL;
			}
			if(obj->TName != L"Just"){
				TESTFAIL;
			}

			if(obj->Prefixes.size() == 2){
				if((*obj->Prefixes[0]) != L"nice wow"){
					TESTFAIL;
				}

				if((*obj->Prefixes[1]) != L"ID(29)"){
					TESTFAIL;
				}

			} else {

				TESTFAIL;
			}

		} else {

			TESTFAIL;
		}
	} else {

		TESTFAIL;
	}

	// cleanup not required //
	// clear old data //
	objects.clear();
	HeaderVars.clear();

	for(int i = 0; i < tests; i++){
		objects = ObjectFileProcessor::ProcessObjectFile(TestFile, HeaderVars);

		objects.clear();
		HeaderVars.clear();
	}

	return Failed;
}

bool TestFloatsCasts(){
	// see if FloatX classes are properly castable //
	Float4 fl(1.f, 2.f, 3.f, 4.f);

	float* ptr = fl;
	// check to  see if values are correct //
	bool Failed = false;

	if((*ptr != fl.X) && (fl[0] == fl.GetX()))
		TESTFAIL;

	// second value //
	if((*(ptr+1) != fl.Y) && (fl[1] == fl.GetY()))
		TESTFAIL;
	// third //
	if((*(ptr+2) != fl.Z) && (fl[2] == fl.GetZ()))
		TESTFAIL;
	// final value //
	if((*(ptr+3) != fl.W) && (fl[3] == fl.GetW()))
		TESTFAIL;

	// check first of UINT4 too //
	UINT4 u4(25, 12, 35, 12);

	UINT* uptr = u4;

	if(*uptr != u4.X)
		TESTFAIL;

	// if got here it succeeded, but return anyways //
	return Failed;
}

bool TestWstringIterator(const int &tests){
	bool Failed = false;


	WstringIterator itr(NULL, false);
	unique_ptr<wstring> results(nullptr);

	// test each one of WstringIterator's get functions and verify that they work correctly //
	itr.ReInit(L" get \" this stuff in here\\\" which has 'stuff' \"_ and not this");


	results = itr.GetStringInQuotes(QUOTETYPE_DOUBLEQUOTES);

	if(*results != L" this stuff in here\\\" which has 'stuff' "){
		TESTFAIL;
	}
	// we should now be on "_" //
	if(itr.GetCurrentCharacter() != L'_'){
		TESTFAIL;
	}

	itr.ReInit(L";quick testcase for \\;not getting anything!");

	//itr.SetDebugMode(true);

	results = itr.GetUntilNextCharacterOrNothing(L';');
	//Logger::Get()->Save();
	if(results->size() > 0){
		TESTFAIL;
	}


	itr.ReInit(L"		teesti_ess y");

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

	if(*results != L"teesti_ess"){
		TESTFAIL;
	}


	itr.ReInit(L" o object type");

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(*results != L"o"){
		TESTFAIL;
	}

	itr.ReInit(L"get-this nice_prefix[but not this!");

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(*results != L"get-this"){
		TESTFAIL;
	}

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

	if(*results != L"nice_prefix"){
		TESTFAIL;
	}


	itr.ReInit(L"aib val: = 243.12al toi() a 2456,12.5");

	results = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
	if(*results != L"243.12"){
		TESTFAIL;
	}

	results = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
	if(*results != L"2456"){
		TESTFAIL;
	}

	results = itr.GetNextNumber(DECIMALSEPARATORTYPE_DOT);
	if(*results != L"12.5"){
		TESTFAIL;
	}

	itr.ReInit(L"	aib val: = 243.12al toi() a 2456,12.5");

	results = itr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_EQUALITY);
	if(*results != L"aib val:"){
		TESTFAIL;
	}

	//itr.SetDebugMode(true);

	itr.ReInit(L"StartCount = [[245]];");

	results = itr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_EQUALITY);
	if(*results != L"StartCount"){
		TESTFAIL;
	}
	itr.SkipWhiteSpace();

	results = itr.GetUntilNextCharacterOrAll(L';');
	Logger::Get()->Save();
	if(*results != L"[[245]]"){
		TESTFAIL;
	}

	itr.ReInit(L" adis told as\\; this still ; and no this");

	results = itr.GetUntilNextCharacterOrNothing(L';');
	if(*results != L" adis told as\\; this still "){
		TESTFAIL;
	}

	itr.ReInit(L"not][ this<out>");

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
	if(*results != L"not"){
		TESTFAIL;
	}

	results = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
	if(*results != L" this"){
		TESTFAIL;
	}

	// some specific cases //
	itr.ReInit(L"\"JellyCube\";");
	results = itr.GetUntilNextCharacterOrAll(L';');
	if(*results != L"\"JellyCube\""){
		TESTFAIL;
	}


	return Failed;
}

//bool TestTextRenderer(const int &tests, Engine* engine){
//
//	bool Failed = false;
//
//	const int TestRenderedID = IDFactory::GetID();
//
//	const Float2 ImaginaryBox(0.8f, 0.4f);
//
//	const wstring texttotest(L"this text should have adjusted length inside box the same as rendered length");
//
//	RenderingFont* font = engine->GetGraphics()->GetTextRenderer()->GetFontFromName(L"Arial");
//
//	size_t FitChar = 0;
//	float EntirelyFit;
//	float Hybrid;
//
//	Float2 AdjustedFinals(0);
//
//	font->AdjustTextSizeToFitBox(ImaginaryBox, texttotest, GUI_POSITIONABLE_COORDTYPE_RELATIVE, FitChar, EntirelyFit, Hybrid, AdjustedFinals, 0.4f);
//
//	wstring adjustedtext(texttotest.size()-1 == FitChar ? texttotest: texttotest.substr(0, 1+FitChar)+L"...");
//
//	size_t FitCharnew = 0;
//
//	float DLLength = font->CalculateDotsSizeAtScale(Hybrid);
//
//
//	float length = font->CalculateTextLengthAndLastFittingExpensive(Hybrid, GUI_POSITIONABLE_COORDTYPE_RELATIVE, adjustedtext, ImaginaryBox.X,
//		FitCharnew, DLLength);
//
//	float cheaplength = font->CalculateTextLengthAndLastFittingNonExpensive(Hybrid, GUI_POSITIONABLE_COORDTYPE_RELATIVE, adjustedtext, ImaginaryBox.X,
//		FitCharnew, DLLength);
//
//	int elength = (int)(length*DataStore::Get()->GetWidth());
//	int clength = (int)(cheaplength*DataStore::Get()->GetWidth());
//
//	Int2 rbox(0);
//	int baselinereceiver;
//	// now render it and check does the length match //
//	font->RenderSentenceToTexture(TestRenderedID, Hybrid, adjustedtext, rbox, baselinereceiver);
//
//	DebugVariableNotifier::PrintVariables();
//
//	Float2 RenderedToBox = Float2(rbox.X/(float)DataStore::Get()->GetWidth(), rbox.Y/(float)DataStore::Get()->GetHeight());
//
//	// check stuff //
//	if(RenderedToBox.X > length){
//
//		Failed = true;
//	}
//
//
//	return Failed;
//}


// --------------------------------------------- //
bool TestFunc(const int &tests){
	bool Failed = false;


	return Failed;
}
