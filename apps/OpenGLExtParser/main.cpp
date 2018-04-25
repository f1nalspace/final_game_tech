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

static const string HeaderIdent = "\t";
static const string BodyIdent = "\t";
static const string StaticDefineName = "FGL_STATIC";
static const string GLApiDefineName = "FGL_GLAPI";
static const string APIEntryDefineName = "FGL_APIENTRY";
static const string GetProcAddressName = "GetOpenGLProcAddress_Internal";

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

inline string ParseHex(const string &str) {
	string result = "";
	for (const char c : str) {
		if (isdigit(c) || ((int)tolower(c) >= 97 && (int)tolower(c) <= 102)) {
			result += c;
		}
	}
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

static GLPrototype ParsePrototype(const string &line, const char *FuncDeclString) {
	GLPrototype result = GLPrototype();

	char *ptr = const_cast<char *>(line.c_str());

	char *name = strstr(ptr, FuncDeclString) + strlen(FuncDeclString) + 1;
	ptr = name;
	SkipWhitespaces(ptr);

	// void *APIENTRY glMapBuffer(GLenum target, GLenum access);
	// const GLubyte *APIENTRY glGetStringi(GLenum name, GLuint index);
	// const GLubyte * APIENTRY glGetString (GLenum name);

	vector<char> prototypeChars = { ' ', '(' };
	while (*ptr) {
		GLString ident = ReadUntilChars(ptr, prototypeChars);
		if (ident.len == 0) break;
		bool incPtr = true;
		if (isspace(*ptr)) {
			if (*(ptr + 1) == '(') {
				ptr++;
			} else if (*(ptr + 1) == '*') {
				ident.len += 2;
				ptr += 2;
				incPtr = false;
			}
		}
		if (*ptr == '(') {
			result.name = string(ident.ptr, ident.len);
			break;
		} else {
			result.returns.emplace_back(string(ident.ptr, ident.len));
		}
		if (incPtr) {
			ptr++;
		} else {
			SkipWhitespaces(ptr);
		}
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

static vector<string> InitGLVersionTypes(const string &version) {
	vector<string> result;
	if (version.compare("GL_VERSION_1_1") == 0) {
		result.push_back(string("typedef unsigned int GLenum;"));
		result.push_back(string("typedef unsigned int GLbitfield;"));
		result.push_back(string("typedef unsigned int GLuint;"));
		result.push_back(string("typedef int GLint;"));
		result.push_back(string("typedef int GLsizei;"));
		result.push_back(string("typedef unsigned char GLboolean;"));
		result.push_back(string("typedef signed char GLbyte;"));
		result.push_back(string("typedef short GLshort;"));
		result.push_back(string("typedef unsigned char GLubyte;"));
		result.push_back(string("typedef unsigned short GLushort;"));
		result.push_back(string("typedef unsigned long GLulong;"));
		result.push_back(string("typedef float GLfloat;"));
		result.push_back(string("typedef float GLclampf;"));
		result.push_back(string("typedef double GLdouble;"));
		result.push_back(string("typedef double GLclampd;"));
		result.push_back(string("typedef void GLvoid;"));
	} else if (version.compare("GL_VERSION_1_5") == 0) {
		result.push_back(string("typedef ptrdiff_t GLsizeiptr;"));
		result.push_back(string("typedef ptrdiff_t GLintptr;"));
	} else if (version.compare("GL_VERSION_2_0") == 0) {
		result.push_back(string("typedef char GLchar;"));
	} else if (version.compare("GL_VERSION_3_2") == 0) {
		result.push_back(string("typedef struct __GLsync *GLsync;"));
		result.push_back(string("typedef uint64_t GLuint64;"));
		result.push_back(string("typedef int64_t GLint64;"));
	} else if (version.compare("GL_VERSION_4_3") == 0) {
		result.push_back(string("typedef void (" + APIEntryDefineName + " *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);"));
	}
	return(result);
}

int main(int argc, char *args[]) {
	if (argc < 2) {
		cerr << "File (glext) argument is missing!" << endl;
		return -1;
	}

	int inputFileCount = argc - 1;

	std::vector<string> inputFilePaths;
	for (int inputFileIndex = 1; inputFileIndex < argc; ++inputFileIndex) {
		inputFilePaths.push_back(string(args[inputFileIndex]));
	}

	vector<string> lines;

	for (int inputFileIndex = 0; inputFileIndex < inputFilePaths.size(); ++inputFileIndex) {
		const string &inputFilePath = inputFilePaths[inputFileIndex];
		cout << "Load '" << inputFilePath << "'" << endl;
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
	}

	string headerOutputFilePath = "header.txt";
	string bodyOutputFilePath = "body.txt";

	cout << "Parse " << inputFileCount << " files with a total of " << lines.size() << " lines" << endl;

	const string ifndefString = "#ifndef ";
	const string endifString = "#endif /* ";

	map<string, vector<GLPrototype>> prototypes;
	map<string, vector<GLConstant>> constants;
	map<string, vector<string>> initTypes;

	size_t lineIndex = 0;
	size_t lineCount = lines.size();
	int64_t glextPrototypesLineIndex = -1;
	int64_t glVersionStartLineIndex = -1;
	string glVersionString = "";
	while (lineIndex < lineCount) {
		string line = lines[lineIndex];

		if (glVersionStartLineIndex == -1) {
			size_t startIndex;
			if ((startIndex = line.find("#ifndef GL_VERSION_", 0)) != string::npos) {
				glVersionStartLineIndex = lineIndex;
				size_t stringIndex = line.find(ifndefString, 0);
				glVersionString = line.substr(stringIndex + ifndefString.size());
				prototypes[glVersionString] = vector<GLPrototype>();
				constants[glVersionString] = vector<GLConstant>();
				initTypes[glVersionString] = InitGLVersionTypes(glVersionString);
			} else if ((startIndex = line.find("#define __gl_h_", 0)) != string::npos) {
				glVersionStartLineIndex = lineIndex;
				glVersionString = "GL_VERSION_1_1";
				prototypes[glVersionString] = vector<GLPrototype>();
				constants[glVersionString] = vector<GLConstant>();
				initTypes[glVersionString] = InitGLVersionTypes(glVersionString);
			}
		} else {
			size_t endIndex;
			if ((endIndex = line.find("#endif /* GL_VERSION_", 0)) != string::npos) {
				glVersionStartLineIndex = -1;
				glVersionString = "";
			} else if ((endIndex = line.find("#endif /* __gl_h_ */", 0)) != string::npos) {
				glVersionStartLineIndex = -1;
				glVersionString = "";
			} else {
				if (line.find("GLAPI ") != string::npos) {
					GLPrototype proto = ParsePrototype(line, "GLAPI");
					prototypes[glVersionString].emplace_back(proto);
				} else if (line.find("WINGDIAPI ") != string::npos) {
					GLPrototype proto = ParsePrototype(line, "WINGDIAPI");
					prototypes[glVersionString].emplace_back(proto);
				} else if (line.find("#define ") != string::npos) {
					GLConstant constant = ParseConstant(line);
					constants[glVersionString].emplace_back(constant);
				}
			}
		}
		++lineIndex;
	}

	cout << "Write header '" << headerOutputFilePath << "'" << endl;


	ofstream headerOutStream = ofstream(headerOutputFilePath, std::ios::out | std::ios::binary);

#define OUTPUT_EXPORT_AS_WELL 0

	for (auto it : prototypes) {
		string version = Trim(it.first);

		headerOutStream << "#" << HeaderIdent << "ifndef " << version << endl;
		headerOutStream << "#" << HeaderIdent << "\tdefine " << version << " 1" << endl;
		headerOutStream << HeaderIdent << "\tstatic bool is" << version << ";" << endl;

		// Typedefs
		const vector<string> initTypeList = initTypes[it.first];
		for (auto initType : initTypeList) {
			headerOutStream << HeaderIdent << "\t" << initType << endl;
		}

		// Constants
		const vector<GLConstant> constantList = constants[it.first];
		for (auto constant : constantList) {
			if (Trim(constant.name) != version && constant.value.size() > 0) {
				headerOutStream << HeaderIdent << "#\tdefine " << constant.name << " " << constant.value << endl;
			}
		}

#if OUTPUT_EXPORT_AS_WELL
		headerOutStream << "#" << HeaderIdent << "\tifdef " << StaticDefineName << endl;
#endif

		const vector<GLPrototype> prototypeList = it.second;
		for (auto prototype : prototypeList) {
			string name = Trim(prototype.name);
			string funcPtrName = MakeFunctionPtrName(name);
			string returnString = "";
			for (auto r : prototype.returns) {
				if (r.find("APIENTRY") != string::npos) {
					break;
				}
				if (returnString.size() > 0) {
					returnString += " ";
				}
				returnString += r;
			}
			string argString = Join(prototype.arguments, ", ", true);
			headerOutStream << HeaderIdent << "\t\ttypedef " << returnString << " (" << APIEntryDefineName << " " << funcPtrName << ")(" << argString << ");" << endl;
			headerOutStream << HeaderIdent << "\t\tstatic " << funcPtrName << "* " << name << ";" << endl;
		}
#if OUTPUT_EXPORT_AS_WELL
		headerOutStream << "#" << HeaderIdent << "\telse" << endl;
		for (auto prototype : prototypeList) {
			string name = Trim(prototype.name);
			string funcPtrName = MakeFunctionPtrName(name);
			string returnString = "";
			for (auto r : prototype.returns) {
				if (r.find("APIENTRY") != string::npos) {
					break;
				}
				if (returnString.size() > 0) {
					returnString += " ";
				}
				returnString += r;
			}
			string argString = Join(prototype.arguments, ", ", true);
			headerOutStream << HeaderIdent << "\t\t" << GLApiDefineName << " " << returnString << " " << APIEntryDefineName << " " << name << "(" << argString << ");" << endl;
		}
		headerOutStream << "#" << HeaderIdent << "\tendif // " << StaticDefineName << endl;
#endif

		headerOutStream << "#" << HeaderIdent << "endif // " << version << endl;
		}

	headerOutStream.flush();
	headerOutStream.close();

	cout << "Write body '" << bodyOutputFilePath << "'" << endl;

	ofstream bodyOutStream = ofstream(bodyOutputFilePath, std::ios::out | std::ios::binary);

	for (auto it : prototypes) {
		string version = Trim(it.first);

		bodyOutStream << "#" << BodyIdent << "if " << version << endl;

		const string VersionString = "GL_VERSION_";
		string versionNumber = version.substr(VersionString.size());

		const vector<GLPrototype> prototypeList = it.second;
		for (auto prototype : prototypeList) {
			string name = Trim(prototype.name);
			string funcPtrName = MakeFunctionPtrName(name);
			bodyOutStream << BodyIdent << "\t" << name << " = (" << funcPtrName << " *)" << GetProcAddressName << "(state, \"" << name << "\");" << endl;
		}

		bodyOutStream << "#" << BodyIdent << "endif //" << version << endl;
		bodyOutStream << endl;
	}

	bodyOutStream.flush();
	bodyOutStream.close();

	return 0;
	}