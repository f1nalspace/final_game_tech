#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <cctype>
#include <inttypes.h>
#include <map>
#include <assert.h>
#include <algorithm> 

using namespace std;

struct GLConstant {
	string name;
	string value;
};

struct GLTypeDefinition {
	vector<string> returns;
	vector<string> arguments;
	string functionOrName;
	bool isFunctionPtr;
};

struct GLPrototype {
	string name;
	vector<string> returns;
	vector<string> arguments;
};

struct GLString {
	char *ptr;
	size_t len;
};

template <typename T, size_t N>
inline size_t ArrayCount(T(&arr)[N]) {
	size_t result = sizeof(arr) / sizeof(arr[0]);
	return(result);
}

inline string Trim(const string& str) {
	size_t first = str.find_first_not_of(' ');
	if (string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

inline string ToUpperCase(const string &str) {
	string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return(result);
}

inline string Join(const vector<string> &list, const char *joinStr, const bool trimValue = false) {
	string result = "";
	size_t idx = 0;
	size_t count = list.size();
	for (const string &str : list) {
		if (idx > 0) {
			result += joinStr;
		}
		if (trimValue)
			result += Trim(str);
		else
			result += str;
		++idx;
	}
	return(result);
}

inline string MakeFunctionPtrName(const string &name) {
	string result = "";
	size_t i = 0;
	size_t count = name.size();
	while (i < count) {
		char c = name[i];
		if (isupper(c) && i < (count - 1)) {
			result += '_';
		}
		result += tolower(c);
		++i;
	}
	result += "_func";
	return(result);
}

inline void SkipWhitespaces(char *&value) {
	if (value != nullptr) {
		while (*value && isspace(*value)) {
			++value;
		}
	}
}

inline GLString ReadIdentifier(char *&value) {
	GLString result = {};
	result.ptr = value;
	if (value != nullptr) {
		while (*value && (isalnum(*value) || (*value == '_'))) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilNonWhitespace(char *&value) {
	GLString result = {};
	result.ptr = value;
	if (value != nullptr) {
		while (*value && !isspace(*value)) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilChar(char *&value, const char search) {
	GLString result = {};
	result.ptr = value;
	if (value != nullptr) {
		while (*value && (*value != search)) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilChars(char *&value, const vector<char> &search) {
	GLString result = {};
	result.ptr = value;
	if (value != nullptr) {
		size_t searchCount = search.size();
		while (*value) {
			bool found = false;
			for (size_t searchIndex = 0; searchIndex < searchCount; ++searchIndex) {
				const char c = search[searchIndex];
				if (*value == c) {
					found = true;
					break;
				}
			}
			if (found) {
				break;
			}
			result.len++;
			++value;
		}
	}
	return(result);
}

static void ParseArguments(char *&ptr, vector<string> &args) {
	// Expect ( = Argument brace start
	assert(*ptr == '(');
	++ptr;

	SkipWhitespaces(ptr);

	// Parse arguments
	while (*ptr) {
		std::vector<char> argChars = { ',', ')' };
		GLString argNameIdent = ReadUntilChars(ptr, argChars);
		if (argNameIdent.len == 0) break;
		args.emplace_back(string(argNameIdent.ptr, argNameIdent.len));
		if (*ptr && *ptr == ',')
			++ptr;
	}

	// Expect ) = Argument brace end
	assert(*ptr == ')');
	++ptr;
}

static void ParseResults(char *&ptr, vector<string> &results) {
	vector<char> charList = { ' ', '(' };
	while (*ptr && *ptr != '(') {
		GLString ident = ReadUntilChars(ptr, charList);
		if (ident.len == 0) break;
		results.emplace_back(string(ident.ptr, ident.len));
		if (*ptr == 0) break;
		if (*ptr == '(') break;
		ptr++;
	}
}

static GLConstant ParseConstant(const string &line) {
	GLConstant result = GLConstant();

	char *ptr = const_cast<char *>(line.c_str());

	const char *DefineString = "#define ";
	char *name = strstr(ptr, DefineString) + strlen(DefineString);
	ptr = name;

	// Skip name but compute name length
	GLString nameIdent = ReadIdentifier(ptr);
	result.name = string(nameIdent.ptr, nameIdent.len);

	SkipWhitespaces(ptr);

	// Parse value
	GLString valueIdent = ReadUntilNonWhitespace(ptr);
	result.value = string(valueIdent.ptr, valueIdent.len);

	return(result);
}

static GLTypeDefinition ParseTypeDefinition(const string &line) {
	GLTypeDefinition result = GLTypeDefinition();

	char *ptr = const_cast<char *>(line.c_str());

	const char *TypeDefString = "typedef ";
	char *name = strstr(ptr, TypeDefString) + strlen(TypeDefString);
	ptr = name;
	SkipWhitespaces(ptr);

	GLString untilSemiliconIdent = ReadUntilChar(ptr, ';');

	string untilSemilicon = string(untilSemiliconIdent.ptr, untilSemiliconIdent.len);
	ptr = const_cast<char *>(untilSemilicon.c_str());

	if ((untilSemilicon.find("(") != string::npos) && (untilSemilicon.find(")") != string::npos)) {
		// Function pointer declaration
		ParseResults(ptr, result.returns);

		assert(*ptr == '(');
		++ptr;
		GLString functionNameIdent = ReadUntilChar(ptr, ')');
		result.functionOrName = string(functionNameIdent.ptr, functionNameIdent.len);
		assert(*ptr == ')');
		++ptr;
		SkipWhitespaces(ptr);

		ParseArguments(ptr, result.arguments);

		result.isFunctionPtr = true;
	} else {
		// Normal typedef
		vector<string> tmp;
		ParseResults(ptr, tmp);
		assert(tmp.size() >= 2);

		result.functionOrName = tmp[tmp.size() - 1];
		result.returns = vector<string>();
		for (int i = 0; i < tmp.size() - 1; ++i) {
			result.returns.emplace_back(tmp[i]);
		}

		result.isFunctionPtr = false;
	}

	return(result);
}

static GLPrototype ParsePrototype(const string &line) {
	GLPrototype result = GLPrototype();

	char *ptr = const_cast<char *>(line.c_str());

	const char *FuncDeclString = "GLAPI ";
	char *name = strstr(ptr, FuncDeclString) + strlen(FuncDeclString);
	ptr = name;
	SkipWhitespaces(ptr);

	vector<char> prototypeChars = { ' ', '(' };
	while (*ptr) {
		GLString ident = ReadUntilChars(ptr, prototypeChars);
		if (ident.len == 0) break;
		if (*ptr == '(') {
			result.name = string(ident.ptr, ident.len);
			break;
		} else {
			result.returns.emplace_back(string(ident.ptr, ident.len));
		}
		ptr++;
	}
	assert(result.name.size() > 0);

	ParseArguments(ptr, result.arguments);

	// Expect ; = Statement end
	assert(*ptr == ';');

	return(result);
}

string ExtractFilenameWithoutExtension(const string &source) {
	string result = "";
	size_t lastIndex = source.find_last_of('.');
	if (lastIndex != string::npos) {
		result = source.substr(0, lastIndex - 1);
	}
	return(result);
}

int main(int argc, char *args[]) {
	if (argc < 2) {
		cerr << "File (glext) argument is missing!" << endl;
		return -1;
	}

	string inputFilePath = args[1];

	vector<string> lines;
	ifstream fileStream = ifstream(inputFilePath, std::ios::in | std::ios::binary);
	if (fileStream.is_open()) {
		for (std::string line; getline(fileStream, line); ) {
			lines.emplace_back(line);
		}
		fileStream.close();
	} else {
		cerr << "File '" << inputFilePath << "' not found!" << endl;
		return -1;
		return -1;
	}

	string filename = ExtractFilenameWithoutExtension(inputFilePath);
	string headerOutputFilePath = filename + "_header.txt";
	string bodyOutputFilePath = filename + "_body.txt";

	cout << "Parse '" << inputFilePath << "'" << endl;

	const string ifndefString = "#ifndef ";
	const string endifString = "#endif /* ";

	map<string, vector<GLPrototype>> prototypes;

	vector<string> outLines;

	size_t lineIndex = 0;
	size_t lineCount = lines.size();
	int64_t glextPrototypesLineIndex = -1;
	int64_t glVersionStartLineIndex = -1;
	string glVersionString = "";
	while (lineIndex < lineCount) {
		string line = lines[lineIndex];

		// Skip prototypes
		if (glextPrototypesLineIndex == -1) {
			size_t startIndex = line.find("#ifdef GL_GLEXT_PROTOTYPES", 0);
			if (startIndex != string::npos) {
				glextPrototypesLineIndex = lineIndex;
				++lineIndex;
				continue;
			}
		} else {
			size_t endIndex = line.find("#endif", 0);
			if (endIndex != string::npos) {
				glextPrototypesLineIndex = -1;
			}
			++lineIndex;
			continue;
		}

		outLines.emplace_back(line);

		if (glVersionStartLineIndex == -1) {
			size_t startIndex = line.find("#ifndef GL_VERSION_", 0);
			if (startIndex != string::npos) {
				glVersionStartLineIndex = lineIndex;
				size_t stringIndex = line.find(ifndefString, 0);
				glVersionString = line.substr(stringIndex + ifndefString.size());
				prototypes[glVersionString] = vector<GLPrototype>();
			}
		} else {
			size_t endIndex = line.find("#endif /* GL_VERSION_", 0);
			if (endIndex != string::npos) {
				glVersionStartLineIndex = -1;
				glVersionString = "";
			} else {
				if (line.find("GLAPI ") != string::npos) {
					GLPrototype proto = ParsePrototype(line);
					prototypes[glVersionString].emplace_back(proto);
				}
			}
		}
		++lineIndex;
	}

	ofstream headerOutStream = ofstream(headerOutputFilePath, std::ios::out | std::ios::binary);

	for (auto line : outLines) {
		headerOutStream << line << endl;
	}

	headerOutStream.flush();
	headerOutStream.close();

	ofstream bodyOutStream = ofstream(bodyOutputFilePath, std::ios::out | std::ios::binary);

#if 0

	bodyOutStream << "//" << endl;
	bodyOutStream << "// Exports Initialization" << endl;
	bodyOutStream << "//" << endl;
	bodyOutStream << endl;
	for (auto funcName : exportFunctions) {
		bodyOutStream << exportName << " " << funcName << " = nullptr;" << endl;
	}

	bodyOutStream << endl;

	bodyOutStream << "//" << endl;
	bodyOutStream << "// Version loader" << endl;
	bodyOutStream << "//" << endl;
	bodyOutStream << "namespace fgll {" << endl;
	vector<string> versionFunctions;
	for (auto it : prototypes) {
		string version = Trim(it.first);

		headerOutStream << "#if " << version << endl;

		const string VersionString = "GL_VERSION_";
		string versionNumber = version.substr(VersionString.size());

		string versionFuncName = "LoadGLVersion" + versionNumber;
		versionFunctions.emplace_back(versionFuncName);
		bodyOutStream << '\t' << exportName << " void " << versionFuncName << "() {" << endl;
		const vector<GLPrototype> prototypeList = it.second;
		for (auto prototype : prototypeList) {
			string name = Trim(prototype.name);
			string funcPtrName = MakeFunctionPtrName(name);
			bodyOutStream << "\t\t" << name << " = (" << funcPtrName << " *)LoadGLFunction(\"" << name << "\");" << endl;
		}
		bodyOutStream << '\t' << "}" << endl;

		headerOutStream << "#if " << version << endl;
		bodyOutStream << endl;
	}

	bodyOutStream << '\t' << exportName << " void LoadAllGLVersions() {" << endl;
	for (auto versionFuncName : versionFunctions) {
		bodyOutStream << "\t\t" << "" << versionFuncName << "();" << endl;
	}
	bodyOutStream << '\t' << "}" << endl;

	bodyOutStream << "};" << endl;

#endif

	bodyOutStream.flush();
	bodyOutStream.close();

	return 0;
}