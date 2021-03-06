
/**
   \file Testing for methods in StringOperations
*/
#include "Common/StringOperations.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("StringOperations::MakeString", "[string]"){

    REQUIRE(sizeof(WINDOWS_LINE_SEPARATOR) - 1 == 2);
    REQUIRE(sizeof(UNIVERSAL_LINE_SEPARATOR) - 1 == 1);

    REQUIRE(WINDOWS_LINE_SEPARATOR[0] == '\r');
    REQUIRE(WINDOWS_LINE_SEPARATOR[1] == '\n');
    REQUIRE(WINDOWS_LINE_SEPARATOR[2] == '\0');

    SECTION("Win separator wstring") {

        std::wstring separator;
        StringOperations::MakeString(separator, WINDOWS_LINE_SEPARATOR,
            sizeof(WINDOWS_LINE_SEPARATOR));

        CHECK(separator == L"\r\n");
    }

    SECTION("Win separator string") {

        std::string separator;
        StringOperations::MakeString(separator, WINDOWS_LINE_SEPARATOR,
            sizeof(WINDOWS_LINE_SEPARATOR));

        CHECK(separator == "\r\n");
    }

}

// First test duplicated for wstring and string others are string only as wstring versions
// *should* also work if they pass
TEST_CASE("Wstring cutting", "[string]"){

	wstring teststring = L"hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! "
        "-qltest -ql-over or not-ql?";
    
	vector<wstring> exampleresult;
	exampleresult.reserve(5);
	exampleresult.push_back(L"hey nice string ");
	exampleresult.push_back(L"you have -qlhere or ");
	exampleresult.push_back(L"mql-yaybe");
	exampleresult.push_back(L" oh no! -qltest ");
	exampleresult.push_back(L"over or not-ql?");

	vector<wstring> result;

	StringOperations::CutString(teststring, wstring(L"-ql-"), result);

    REQUIRE(result.size() == exampleresult.size());
    
    for(size_t i = 0; i < exampleresult.size(); i++){

        REQUIRE(result[i] == exampleresult[i]);
    }
}

TEST_CASE("String cutting", "[string]"){

	string teststring = "hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! "
        "-qltest -ql-over or not-ql?";
    
	vector<string> exampleresult;
	exampleresult.reserve(5);
	exampleresult.push_back("hey nice string ");
	exampleresult.push_back("you have -qlhere or ");
	exampleresult.push_back("mql-yaybe");
	exampleresult.push_back(" oh no! -qltest ");
	exampleresult.push_back("over or not-ql?");

	vector<string> result;

	StringOperations::CutString(teststring, string("-ql-"), result);

    REQUIRE(result.size() == exampleresult.size());
    
    for(size_t i = 0; i < exampleresult.size(); i++){

        REQUIRE(result[i] == exampleresult[i]);
    }

    SECTION("Single value gets copied to output"){

        std::vector<std::string> tagparts;
        StringOperations::CutString<std::string>("tag", ";", tagparts);
    
        REQUIRE(tagparts.size() == 1);
        CHECK(tagparts[0] == "tag");
    }

    SECTION("Single character line cutting"){

        std::vector<std::string> result;

        StringOperations::CutLines<std::string>("victory sign\npeace sign\nv", result);

        CHECK(result == std::vector<std::string>({"victory sign", "peace sign", "v"}));
    }

    SECTION("Empty CutString"){

        std::vector<std::string> result;

        CHECK(StringOperations::CutString<std::string>("", "a", result) == false);

        REQUIRE(result.size() == 1);
        CHECK(result[0] == "");
    }
}

TEST_CASE("String text replacing ", "[string]"){

    SECTION("Nice spaced out delimiters"){
        const string teststring = "hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! -qltest -ql-over "
            "or not-ql?";
        const string correctresult = "hey nice string hey_whatsthis?you have -qlhere or hey_whatsthis?mql-yaybehey_"
            "whatsthis? oh no! -qltest hey_whatsthis?over or not-ql?";

        string result = StringOperations::Replace<string>(teststring, "-ql-", "hey_whatsthis?");

        REQUIRE(result == correctresult);

    }

    SECTION("Replace after first character"){

        CHECK(StringOperations::Replace<std::string>("u%2FCute%20L%2F66%2F153370%2F01.jpg",
                "%2F", "/") == "u/Cute%20L/66/153370/01.jpg");
    }
    
}


TEST_CASE("String whitespace trimming", "[string]"){

    const string testdata = "		a  asd	hey nice   ?!	 ";

	string result = testdata;
	const string correctresult = "a  asd	hey nice   ?!";

	StringOperations::RemovePreceedingTrailingSpaces(result);

    REQUIRE(result == correctresult);
}

TEST_CASE("String whitespace trimming without any whitespace", "[string]"){

    SECTION("Single letter"){
        string result = "s";
        StringOperations::RemovePreceedingTrailingSpaces(result);

        CHECK(result == "s");
    }

    SECTION("Short word"){

        string result = "sam";
        StringOperations::RemovePreceedingTrailingSpaces(result);

        CHECK(result == "sam");
    }
}

#define TESTPATHPLAIN	"My/super/path/filesthis.extsuper"

#define TESTPATHSTR			"My/super/path/filesthis.extsuper"
#define TESTPATHSTRWIN		"My\\super\\path\\filesthis.extsuper"


TEST_CASE("StringOperations common work with string and wstring", "[string]"){

	wstring paththing = Convert::Utf8ToUtf16(TESTPATHPLAIN);
	string paththing2 = TESTPATHSTRWIN;


	wstring result = StringOperations::RemoveExtension<wstring>(paththing, true);
	string result2 = StringOperations::RemoveExtension<string>(paththing2, true);

	wstring wstringified = Convert::Utf8ToUtf16(result2);

    CHECK(result == wstringified);
    CHECK(result == L"filesthis");

	result = StringOperations::GetExtension<wstring>(paththing);

	CHECK(result == L"extsuper");

	result = StringOperations::ChangeExtension<wstring>(paththing, L"superier");

	CHECK(result == L"My/super/path/filesthis.superier");

	wstring ressecond = StringOperations::RemovePath<wstring>(result);

	CHECK(ressecond == L"filesthis.superier");

	string removed = StringOperations::RemovePath<std::string>(
        "./GUI/Nice/Panels/Mytexture.png");

	CHECK(removed == "Mytexture.png");

	wstring pathy = StringOperations::GetPath<std::wstring>(paththing);

	CHECK(pathy == L"My/super/path/");

	CHECK(StringOperations::StringStartsWith(wstring(L"My super text"), wstring(L"My")));
    // This line causes use of uninitialized value error
    // bool val = StringOperations::StringStartsWith(wstring(L"}"), wstring(L"}"));
    // CHECK(val);
    CHECK(StringOperations::StringStartsWith(std::string("}"), std::string("}")));
    CHECK(!StringOperations::StringStartsWith(string("This shouldn't match"),
            string("this")));

	// Line end changing //
    wstring simplestr = L"Two\nlines";

    const wstring convresult = StringOperations::ChangeLineEndsToWindows<std::wstring>(
        simplestr);

    CHECK(convresult == L"Two\r\nlines");

	wstring pathtestoriginal = L"My text is quite nice\nand has\n multiple\r\n lines\n"
        L"that are separated\n";

	wstring pathresult = StringOperations::ChangeLineEndsToWindows<std::wstring>(
        pathtestoriginal);

	CHECK(pathresult == L"My text is quite nice\r\nand has\r\n multiple\r\n lines\r\nthat "
        L"are separated\r\n");

	wstring backlinetest = StringOperations::ChangeLineEndsToUniversal<std::wstring>(
        pathresult);

	CHECK(backlinetest == L"My text is quite nice\nand has\n multiple\n lines\nthat are "
        L"separated\n");
}

TEST_CASE("StringOperations indent creation", "[string]") {

    CHECK(StringOperations::Indent<std::string>(1) == " ");
    CHECK(StringOperations::Indent<std::string>(3) == "   ");

    CHECK(StringOperations::Indent<std::wstring>(1) == L" ");
    CHECK(StringOperations::Indent<std::wstring>(3) == L"   ");
}

TEST_CASE("StringOperations indent lines", "[string]") {

    SECTION("Single line") {

        CHECK(StringOperations::IndentLines<std::string>("this is a line", 2) ==
            "  this is a line");
    }

    SECTION("Two lines") {

        CHECK(StringOperations::IndentLines<std::string>("this is a line\nthis is a second",
                1) == " this is a line\n this is a second");
    }

    SECTION("Ends with a new line") {

        CHECK(StringOperations::IndentLines<std::string>("this is a line\n", 1) ==
            " this is a line\n");
    }

    SECTION("Remove existing spaces") {

        SECTION("Single line") {

            CHECK(StringOperations::IndentLines<std::string>(" this is a line", 2) ==
                "  this is a line");
        }

        SECTION("Two lines") {

            CHECK(StringOperations::IndentLines<std::string>(
                    "    this is a line\n  this is a second", 1) ==
                " this is a line\n this is a second");
        }
    }

    SECTION("Basic \\n lines") {

        constexpr auto input = "this is a\n multiline story\nthat spans many lines\n";
        constexpr auto result = "   this is a\n   multiline story\n   that spans many lines\n";

        CHECK(StringOperations::IndentLines<std::string>(input, 3) == result);
    }

    SECTION("Windows lines") {

        constexpr auto input = "this is a\r\n multiline story\r\nthat spans many lines\r\n";
        constexpr auto result = "   this is a\n   multiline story\n   that spans many lines\n";

        CHECK(StringOperations::IndentLines<std::string>(input, 3) == result);
    }
}

TEST_CASE("StringOperations replace sha hash character", "[string]") {

    CHECK(StringOperations::Replace<std::string>(
            "II+O7pSQgH8BG/gWrc+bAetVgxJNrJNX4zhA4oWV+V0=", "/", "_") ==
        "II+O7pSQgH8BG_gWrc+bAetVgxJNrJNX4zhA4oWV+V0=");

    CHECK(StringOperations::ReplaceSingleCharacter<std::string>(
            "II+O7pSQgH8BG/gWrc+bAetVgxJNrJNX4zhA4oWV+V0=", "/", '_') ==
        "II+O7pSQgH8BG_gWrc+bAetVgxJNrJNX4zhA4oWV+V0=");
    
}

TEST_CASE("StringOperations cut on new line", "[string]") {

    SECTION("Single line"){

        std::vector<std::string> output;

        CHECK(StringOperations::CutLines<std::string>("just a single string", output) == 1);

        REQUIRE(!output.empty());
        CHECK(output[0] == "just a single string");
    }

    SECTION("Basic two lines"){

        std::vector<std::string> output;

        CHECK(StringOperations::CutLines<std::string>("this is\n two lines", output) == 2);

        REQUIRE(output.size() == 2);
        CHECK(output[0] == "this is");
        CHECK(output[1] == " two lines");
    }

    SECTION("Windows separator"){

        std::vector<std::string> output;

        CHECK(StringOperations::CutLines<std::string>("this is\r\n two lines", output) == 2);

        REQUIRE(output.size() == 2);
        CHECK(output[0] == "this is");
        CHECK(output[1] == " two lines");
    }

    SECTION("Ending with a line separator"){

        std::vector<std::string> output;

        CHECK(StringOperations::CutLines<std::string>("this is\n two lines\n", output) == 2);

        REQUIRE(output.size() == 2);
        CHECK(output[0] == "this is");
        CHECK(output[1] == " two lines");
    }

    SECTION("Counting lines"){

        SECTION("Without empty lines"){

            std::vector<std::string> output;

            CHECK(StringOperations::CutLines<std::string>("just put \nsome line\nseparators "
                    "in here to\ncheck", output) == 4);

            REQUIRE(output.size() == 4);
        }
        
        SECTION("With empty lines"){

            SECTION("\\n"){
                std::vector<std::string> output;

                CHECK(StringOperations::CutLines<std::string>("just put \n\nseparators "
                        "in here to\ncheck", output) == 4);

                REQUIRE(output.size() == 4);
            }

            SECTION("Windows separator"){

                std::vector<std::string> output;

                CHECK(StringOperations::CutLines<std::string>("just put \r\n\r\nseparators "
                        "in here to\r\ncheck", output) == 4);

                REQUIRE(output.size() == 4);
            }
        }
    }
}

TEST_CASE("StringOperations remove characters", "[string]") {

    SECTION("Nothing gets removed"){

        CHECK(StringOperations::RemoveCharacters<std::string>("just a single string", "") ==
            "just a single string");

        CHECK(StringOperations::RemoveCharacters<std::string>("just a single string", "z") ==
            "just a single string");
        
    }

    SECTION("Single character"){
        
        CHECK(StringOperations::RemoveCharacters<std::string>("just a single string", " ") ==
            "justasinglestring");

        CHECK(StringOperations::RemoveCharacters<std::string>("just a single string", "i") ==
            "just a sngle strng");
    }

    SECTION("Multiple characters"){

        CHECK(StringOperations::RemoveCharacters<std::string>("just a single string", " i") ==
            "justasnglestrng");
    }
}

TEST_CASE("StringOperations URL combine", "[string][url]"){

    SECTION("Hostnames"){
        
        CHECK(StringOperations::BaseHostName("http://google.fi") == "http://google.fi/");

        CHECK(StringOperations::BaseHostName("http://google.fi/") == "http://google.fi/");


        CHECK(StringOperations::BaseHostName(
                "http://a.content.com/b/Word Space/664/10232/01.jpg") ==
            "http://a.content.com/");
    }

    SECTION("Get protocol"){

        CHECK(StringOperations::URLProtocol("http://google.fi/") == "http");
        CHECK(StringOperations::URLProtocol("https://google.fi/") == "https");
        CHECK(StringOperations::URLProtocol("tel:936704") == "tel");
        CHECK(StringOperations::URLProtocol("telnet://site.com") == "telnet");
    }

    SECTION("Combines"){
        CHECK(StringOperations::CombineURL("http://google.fi", "img.jpg") ==
            "http://google.fi/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi/", "img.jpg") ==
            "http://google.fi/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi/", "/img.jpg") ==
            "http://google.fi/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi", "/img.jpg") ==
            "http://google.fi/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi/index.html", "/img.jpg") ==
            "http://google.fi/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi/index.html/", "img.jpg") ==
            "http://google.fi/index.html/img.jpg");

        CHECK(StringOperations::CombineURL("http://google.fi/index.html", "/other/img.jpg") ==
            "http://google.fi/other/img.jpg");
    }

    SECTION("combine with double /"){
        CHECK(StringOperations::CombineURL(
                "https://example.com/index.php?page=img",
                "https://example.com//img.example.com//images/1234.jpeg") ==
            "https://img.example.com/images/1234.jpeg");
    }
}

TEST_CASE("StringOperations IsURLDomain", "[string][url]"){

    CHECK(StringOperations::IsURLDomain("example.com"));
    CHECK(StringOperations::IsURLDomain("google.com"));
    CHECK(StringOperations::IsURLDomain("boostslair.com"));
    CHECK(StringOperations::IsURLDomain("fp.boostslair.com"));
    CHECK(StringOperations::IsURLDomain("a.b.test.com"));
    CHECK(!StringOperations::IsURLDomain("a.b/test.com"));
    CHECK(!StringOperations::IsURLDomain("file"));
}

TEST_CASE("StringOperations URL cut to path", "[string][url]"){

    CHECK(StringOperations::URLPath("http://google.fi/index.html") == "index.html");


    CHECK(StringOperations::URLPath("http://a.content.com/b/Word Space/664/10232/01.jpg") ==
        "b/Word Space/664/10232/01.jpg");

 
}    

TEST_CASE("StringOperations RepeatCharacter", "[string]"){

    SECTION("Basic one byte characters"){

        CHECK(StringOperations::RepeatCharacter<std::string>('a', 4) == "aaaa");
        CHECK(StringOperations::RepeatCharacter<std::string>('a', 0) == "");
        CHECK(StringOperations::RepeatCharacter<std::string>('a', 3) == "aaa");
        CHECK(StringOperations::RepeatCharacter<std::string>(' ', 4) == "    ");
    }
}

TEST_CASE("StringOperations RemoveEnding", "[string]"){

    SECTION("Single character"){

        CHECK(StringOperations::RemoveEnding<std::string>("my string", "g") == "my strin");
    }

    SECTION("Short string"){

        CHECK(StringOperations::RemoveEnding<std::string>("my string that ends with this",
                " this") == "my string that ends with");
    }

    SECTION("Not removing anything"){

        CHECK(StringOperations::RemoveEnding<std::string>("my string", "n") == "my string");

        CHECK(StringOperations::RemoveEnding<std::string>("my string that ends with this",
                "i") == "my string that ends with this");
    }
}

