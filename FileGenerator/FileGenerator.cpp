#include "FileGenerator.h"
#include "JSMin.h"

#include <boost/filesystem.hpp>

#include <iosfwd>


bool FileGenerator::DoJSExtensionGeneration(
    string input, string output, const std::string& name)
{

    // There should be no '"' characters //

    // Create folders
    boost::filesystem::create_directories(boost::filesystem::path(output).parent_path());

    // Write stuff over the file //
    ofstream writer(output);
    ifstream reader(input);

    if(!reader.is_open()) {

        cout << "Failed to open input file:" << input << endl;
        return false;
    }

    if(!writer.is_open()) {

        cout << "Failed to open output file:" << output << endl;
        return false;
    }

    const auto lastSlash = input.find_last_of('/') + 1;
    const auto lastDot = input.find_last_of('.') - 1;
    const auto inputName = input.substr(lastSlash, lastDot - lastSlash + 1);

    // Write the heading stuff //
    writer << "#pragma once" << endl << endl;
    writer << "// Generated by Leviathan FileGenerator for " << name << ", using JSMin "
              "Copyright (c) 2002 Douglas Crockford //"
           << endl;
    writer << "// From: " << input << " //" << endl;
    writer << "namespace " << name << "{" << endl;
    writer << "const char* " << inputName << "Str = \"\"" << endl;

    // Read from the input and put to output //
    char buff[400];
    string filetext = "";

    while(reader.good()) {

        reader.getline(buff, 400);

        filetext += "\n" + string(buff);
    }

    reader.close();

    // The file is now read, minimize it //
    JSMin minifier;

    minifier.SetInput(filetext);


    try {
        minifier.jsmin();
    } catch(...) {
        // Probably is bad //
        cout << "Parsing error" << endl;
        return false;
    }

    stringstream minstr(minifier.GetOutputString());

    // stringstream minstr(filetext);

    while(minstr.good()) {

        minstr.getline(buff, 400);

        string line = buff;

        if(line.size() == 0)
            continue;

        writer << "\"";
        // Quite an ugly hack to remove all quotes //
        for(size_t i = 0; i < line.length(); i++) {
            if(line[i] == '"') {
                writer << "\\\"";
                continue;
            }
            writer << line[i];
        }


        // Write the final line //
        writer << "\\n\"" << endl;
    }


    writer << endl << ";";
    writer << endl << "}";

    writer.close();

    cout << "Done generating file: " << output << endl;
    return true;
}
