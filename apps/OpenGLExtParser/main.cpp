/*
-------------------------------------------------------------------------------
# Name
	OpenGL Header Parser/Generator.	For internal usage only!
	Version 1.1

# Author
	Torsten Spaete
	Copyright (C) 

# Requirements
	- C++/11 Compiler

# Changelog
	## v1.1
	- Generating better output to support extern

	## v1.0:
	- Initial creation

# License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

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

static const int HeaderIdent = 1;
static const int LoaderIdent = 1;
static const int VariablesIdent = 2;

// Defines for final_dynamic_opengl.h
static const char *OneTab = "\t";
static const char *ApiName = "fgl_api";
static const char *FunctionPrefix = "fgl_";
static const char *GLApiDefineName = "FGL_GLAPI";
static const char *APIEntryDefineName = "FGL_APIENTRY";
static const char *GetProcAddressName = "fgl__GetOpenGLProcAddress";
static const char *NullName = "fgl_null";

struct GLConstant {
	std::string name;
	std::string value;
};

struct GLTypeDefinition {
	std::vector<std::string> returns;
	std::vector<std::string> arguments;
	std::string functionOrName;
	bool isFunctionPtr;
};

struct GLPrototype {
	std::string name;
	std::vector<std::string> returns;
	std::vector<std::string> arguments;
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

inline std::string Trim(const std::string &str) {
	size_t first = str.find_first_not_of(' ');
	if(std::string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

inline std::string ToUpperCase(const std::string &str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return(result);
}

inline std::string ParseHex(const std::string &str) {
	std::string result = "";
	for(const char c : str) {
		if(isdigit(c) || ((int)tolower(c) >= 97 && (int)tolower(c) <= 102)) {
			result += c;
		}
	}
	return(result);
}

inline std::string Join(const std::vector<std::string> &list, const char *joinStr, const bool trimValue = false) {
	std::string result = "";
	size_t idx = 0;
	size_t count = list.size();
	for(const std::string &str : list) {
		if(idx > 0) {
			result += joinStr;
		}
		if(trimValue)
			result += Trim(str);
		else
			result += str;
		++idx;
	}
	return(result);
}

inline std::string MakeFunctionPtrName(const std::string &prefix, const std::string &name) {
	std::string result = prefix + "func_" + name;
	return(result);
}

inline void SkipWhitespaces(char *&value) {
	if(value != nullptr) {
		while(*value && isspace(*value)) {
			++value;
		}
	}
}

inline GLString ReadIdentifier(char *&value) {
	GLString result = {};
	result.ptr = value;
	if(value != nullptr) {
		while(*value && (isalnum(*value) || (*value == '_'))) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilNonWhitespace(char *&value) {
	GLString result = {};
	result.ptr = value;
	if(value != nullptr) {
		while(*value && !isspace(*value)) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilChar(char *&value, const char search) {
	GLString result = {};
	result.ptr = value;
	if(value != nullptr) {
		while(*value && (*value != search)) {
			result.len++;
			++value;
		}
	}
	return(result);
}

inline GLString ReadUntilChars(char *&value, const std::vector<char> &search) {
	GLString result = {};
	result.ptr = value;
	if(value != nullptr) {
		size_t searchCount = search.size();
		while(*value) {
			bool found = false;
			for(size_t searchIndex = 0; searchIndex < searchCount; ++searchIndex) {
				const char c = search[searchIndex];
				if(*value == c) {
					found = true;
					break;
				}
			}
			if(found) {
				break;
			}
			result.len++;
			++value;
		}
	}
	return(result);
}

static void ParseArguments(char *&ptr, std::vector<std::string> &args) {
	// Expect ( = Argument brace start
	assert(*ptr == '(');
	++ptr;

	SkipWhitespaces(ptr);

	// Parse arguments
	while(*ptr) {
		std::vector<char> argChars = { ',', ')' };
		GLString argNameIdent = ReadUntilChars(ptr, argChars);
		if(argNameIdent.len == 0) break;
		args.emplace_back(std::string(argNameIdent.ptr, argNameIdent.len));
		if(*ptr && *ptr == ',')
			++ptr;
	}

	// Expect ) = Argument brace end
	assert(*ptr == ')');
	++ptr;
}

static void ParseResults(char *&ptr, std::vector<std::string> &results) {
	std::vector<char> charList = { ' ', '(' };
	while(*ptr && *ptr != '(') {
		GLString ident = ReadUntilChars(ptr, charList);
		if(ident.len == 0) break;
		results.emplace_back(std::string(ident.ptr, ident.len));
		if(*ptr == 0) break;
		if(*ptr == '(') break;
		ptr++;
	}
}

static GLConstant ParseConstant(const std::string &line) {
	GLConstant result = GLConstant();

	char *ptr = const_cast<char *>(line.c_str());

	const char *DefineString = "#define ";
	char *name = strstr(ptr, DefineString) + strlen(DefineString);
	ptr = name;

	// Skip name but compute name length
	GLString nameIdent = ReadIdentifier(ptr);
	result.name = std::string(nameIdent.ptr, nameIdent.len);

	SkipWhitespaces(ptr);

	// Parse value
	GLString valueIdent = ReadUntilNonWhitespace(ptr);
	result.value = std::string(valueIdent.ptr, valueIdent.len);

	return(result);
}

static GLTypeDefinition ParseTypeDefinition(const std::string &line) {
	GLTypeDefinition result = GLTypeDefinition();

	char *ptr = const_cast<char *>(line.c_str());

	const char *TypeDefString = "typedef ";
	char *name = strstr(ptr, TypeDefString) + strlen(TypeDefString);
	ptr = name;
	SkipWhitespaces(ptr);

	GLString untilSemiliconIdent = ReadUntilChar(ptr, ';');

	std::string untilSemilicon = std::string(untilSemiliconIdent.ptr, untilSemiliconIdent.len);
	ptr = const_cast<char *>(untilSemilicon.c_str());

	if((untilSemilicon.find("(") != std::string::npos) && (untilSemilicon.find(")") != std::string::npos)) {
		// Function pointer declaration
		ParseResults(ptr, result.returns);

		assert(*ptr == '(');
		++ptr;
		GLString functionNameIdent = ReadUntilChar(ptr, ')');
		result.functionOrName = std::string(functionNameIdent.ptr, functionNameIdent.len);
		assert(*ptr == ')');
		++ptr;
		SkipWhitespaces(ptr);

		ParseArguments(ptr, result.arguments);

		result.isFunctionPtr = true;
	} else {
		// Normal typedef
		std::vector<std::string> tmp;
		ParseResults(ptr, tmp);
		assert(tmp.size() >= 2);

		result.functionOrName = tmp[tmp.size() - 1];
		result.returns = std::vector<std::string>();
		for(int i = 0; i < tmp.size() - 1; ++i) {
			result.returns.emplace_back(tmp[i]);
		}

		result.isFunctionPtr = false;
	}

	return(result);
}

static GLPrototype ParsePrototype(const std::string &line, const char *FuncDeclString) {
	GLPrototype result = GLPrototype();

	char *ptr = const_cast<char *>(line.c_str());

	char *name = strstr(ptr, FuncDeclString) + strlen(FuncDeclString) + 1;
	ptr = name;
	SkipWhitespaces(ptr);

	// void *APIENTRY glMapBuffer(GLenum target, GLenum access);
	// const GLubyte *APIENTRY glGetStringi(GLenum name, GLuint index);
	// const GLubyte * APIENTRY glGetString (GLenum name);

	std::vector<char> prototypeChars = { ' ', '(' };
	while(*ptr) {
		GLString ident = ReadUntilChars(ptr, prototypeChars);
		if(ident.len == 0) break;
		bool incPtr = true;
		if(isspace(*ptr)) {
			if(*(ptr + 1) == '(') {
				ptr++;
			} else if(*(ptr + 1) == '*') {
				ident.len += 2;
				ptr += 2;
				incPtr = false;
			}
		}
		if(*ptr == '(') {
			result.name = std::string(ident.ptr, ident.len);
			break;
		} else {
			result.returns.emplace_back(std::string(ident.ptr, ident.len));
		}
		if(incPtr) {
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

std::string ExtractFilenameWithoutExtension(const std::string &source) {
	std::string result = "";
	size_t lastIndex = source.find_last_of('.');
	if(lastIndex != std::string::npos) {
		result = source.substr(0, lastIndex - 1);
	}
	return(result);
}

static std::vector<std::string> InitGLVersionTypes(const std::string &version) {
	std::vector<std::string> result;
	if(version.compare("GL_VERSION_1_1") == 0) {
		result.push_back(std::string("typedef unsigned int GLenum;"));
		result.push_back(std::string("typedef unsigned int GLbitfield;"));
		result.push_back(std::string("typedef unsigned int GLuint;"));
		result.push_back(std::string("typedef int GLint;"));
		result.push_back(std::string("typedef int GLsizei;"));
		result.push_back(std::string("typedef unsigned char GLboolean;"));
		result.push_back(std::string("typedef signed char GLbyte;"));
		result.push_back(std::string("typedef short GLshort;"));
		result.push_back(std::string("typedef unsigned char GLubyte;"));
		result.push_back(std::string("typedef unsigned short GLushort;"));
		result.push_back(std::string("typedef unsigned long GLulong;"));
		result.push_back(std::string("typedef float GLfloat;"));
		result.push_back(std::string("typedef float GLclampf;"));
		result.push_back(std::string("typedef double GLdouble;"));
		result.push_back(std::string("typedef double GLclampd;"));
		result.push_back(std::string("typedef void GLvoid;"));
	} else if(version.compare("GL_VERSION_1_5") == 0) {
		result.push_back(std::string("typedef ptrdiff_t GLsizeiptr;"));
		result.push_back(std::string("typedef ptrdiff_t GLintptr;"));
	} else if(version.compare("GL_VERSION_2_0") == 0) {
		result.push_back(std::string("typedef char GLchar;"));
	} else if(version.compare("GL_VERSION_3_2") == 0) {
		result.push_back(std::string("typedef struct __GLsync *GLsync;"));
		result.push_back(std::string("typedef uint64_t GLuint64;"));
		result.push_back(std::string("typedef int64_t GLint64;"));
	} else if(version.compare("GL_VERSION_4_3") == 0) {
		result.push_back(std::string("typedef void (" + std::string(APIEntryDefineName) + " *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);"));
	}
	return(result);
}

std::string GetTabbedString(const int count) {
	std::string result = "";
	for(int i = 0; i < count; ++i) {
		result += OneTab;
	}
	return(result);
}

int main(int argc, char *args[]) {
	if(argc < 3) {
		std::cerr << "Files (gl.h and glext.h) arguments are missing!" << std::endl;
		return -1;
	}

	int inputFileCount = argc - 1;

	std::vector<std::string> inputFilePaths;
	for(int inputFileIndex = 1; inputFileIndex < argc; ++inputFileIndex) {
		inputFilePaths.push_back(std::string(args[inputFileIndex]));
	}

	std::vector<std::string> lines;

	for(int inputFileIndex = 0; inputFileIndex < inputFilePaths.size(); ++inputFileIndex) {
		const std::string &inputFilePath = inputFilePaths[inputFileIndex];
		std::cout << "Load '" << inputFilePath << "'" << std::endl;
		std::ifstream fileStream = std::ifstream(inputFilePath, std::ios::in | std::ios::binary);
		if(fileStream.is_open()) {
			for(std::string line; getline(fileStream, line); ) {
				lines.emplace_back(line);
			}
			fileStream.close();
		} else {
			std::cerr << "File '" << inputFilePath << "' not found!" << std::endl;
			return -1;
			return -1;
		}
	}

	std::string headerOutputFilePath = "header.txt";
	std::string bodyOutputFilePath = "body.txt";

	std::cout << "Parse " << inputFileCount << " files with a total of " << lines.size() << " lines" << std::endl;

	const std::string ifndefString = "#ifndef ";
	const std::string endifString = "#endif /* ";

	std::map<std::string, std::vector<GLPrototype>> prototypes;
	std::map<std::string, std::vector<GLConstant>> constants;
	std::map<std::string, std::vector<std::string>> initTypes;

	size_t lineIndex = 0;
	size_t lineCount = lines.size();
	int64_t glextPrototypesLineIndex = -1;
	int64_t glVersionStartLineIndex = -1;
	std::string glVersionString = "";
	while(lineIndex < lineCount) {
		std::string line = lines[lineIndex];

		if(glVersionStartLineIndex == -1) {
			size_t startIndex;
			if((startIndex = line.find("#ifndef GL_VERSION_", 0)) != std::string::npos) {
				glVersionStartLineIndex = lineIndex;
				size_t stringIndex = line.find(ifndefString, 0);
				glVersionString = line.substr(stringIndex + ifndefString.size());
				prototypes[glVersionString] = std::vector<GLPrototype>();
				constants[glVersionString] = std::vector<GLConstant>();
				initTypes[glVersionString] = InitGLVersionTypes(glVersionString);
			} else if((startIndex = line.find("#define __gl_h_", 0)) != std::string::npos) {
				glVersionStartLineIndex = lineIndex;
				glVersionString = "GL_VERSION_1_1";
				prototypes[glVersionString] = std::vector<GLPrototype>();
				constants[glVersionString] = std::vector<GLConstant>();
				initTypes[glVersionString] = InitGLVersionTypes(glVersionString);
			}
		} else {
			size_t endIndex;
			if((endIndex = line.find("#endif /* GL_VERSION_", 0)) != std::string::npos) {
				glVersionStartLineIndex = -1;
				glVersionString = "";
			} else if((endIndex = line.find("#endif /* __gl_h_ */", 0)) != std::string::npos) {
				glVersionStartLineIndex = -1;
				glVersionString = "";
			} else {
				if(line.find("GLAPI ") != std::string::npos) {
					GLPrototype proto = ParsePrototype(line, "GLAPI");
					prototypes[glVersionString].emplace_back(proto);
				} else if(line.find("WINGDIAPI ") != std::string::npos) {
					GLPrototype proto = ParsePrototype(line, "WINGDIAPI");
					prototypes[glVersionString].emplace_back(proto);
				} else if(line.find("#define ") != std::string::npos) {
					GLConstant constant = ParseConstant(line);
					constants[glVersionString].emplace_back(constant);
				}
			}
		}
		++lineIndex;
	}

	//
	// Write Header
	//
	std::string hident0 = GetTabbedString(HeaderIdent);
	std::cout << "Write header '" << headerOutputFilePath << "'" << std::endl;
	std::ofstream headerOutStream = std::ofstream(headerOutputFilePath, std::ios::out | std::ios::binary);
	headerOutStream << hident0 << "// *******************************************************************************" << std::endl;
	headerOutStream << hident0 << "//" << std::endl;
	headerOutStream << hident0 << "// > OpenGL Header" << std::endl;
	headerOutStream << hident0 << "//" << std::endl;
	headerOutStream << hident0 << "// Automatically generated. Do not modify by hand!" << std::endl;
	headerOutStream << hident0 << "//" << std::endl;
	headerOutStream << hident0 << "// *******************************************************************************" << std::endl;
	for(auto it : prototypes) {
		std::string version = Trim(it.first);

		std::string ident0 = GetTabbedString(HeaderIdent);
		std::string ident1 = GetTabbedString(HeaderIdent + 1);

		headerOutStream << "#" << ident0 << "ifndef " << version << std::endl;

		headerOutStream << "#" << ident1 << "define " << version << " 1" << std::endl;
		headerOutStream << ident1 << ApiName << " bool is" << version << ";" << std::endl;
		headerOutStream << std::endl;

		// Typedefs
		const std::vector<std::string> initTypeList = initTypes[it.first];
		for(auto initType : initTypeList) {
			headerOutStream << ident1 << initType << std::endl;
		}
		if(initTypeList.size() > 0) {
			headerOutStream << std::endl;
		}

		// Constants
		const std::vector<GLConstant> constantList = constants[it.first];
		for(auto constant : constantList) {
			if(Trim(constant.name) != version && constant.value.size() > 0) {
				headerOutStream << "#" << ident1 << "define " << constant.name << " " << constant.value << std::endl;
			}
		}
		if(constantList.size() > 0) {
			headerOutStream << std::endl;
		}

		const std::vector<GLPrototype> prototypeList = it.second;
		for(auto prototype : prototypeList) {
			const std::string originalName = Trim(prototype.name);
			std::string returnString = "";
			for(auto r : prototype.returns) {
				if(r.find("APIENTRY") != std::string::npos) {
					break;
				}
				if(returnString.size() > 0) {
					returnString += " ";
				}
				returnString += r;
			}

			std::string typedefName = MakeFunctionPtrName(FunctionPrefix, originalName);
			std::string staticFunctionName = FunctionPrefix + originalName;
			std::string defineFunctionName = originalName;

			std::string argString = Join(prototype.arguments, ", ", true);
			headerOutStream << ident1 << "typedef " << returnString << " (" << APIEntryDefineName << " " << typedefName << ")(" << argString << ");" << std::endl;
			headerOutStream << ident1 << ApiName << " " << typedefName << "* " << staticFunctionName << ";" << std::endl;
			headerOutStream << "#" << ident1 << "define " << defineFunctionName << " " << staticFunctionName << std::endl;
		}

		headerOutStream << "#" << ident0 << "endif // " << version << std::endl;
		headerOutStream << std::endl;
	}
	headerOutStream.flush();
	headerOutStream.close();

	//
	// Write Body
	//
	std::string vident0 = GetTabbedString(VariablesIdent);
	std::cout << "Write body '" << bodyOutputFilePath << "'" << std::endl;
	std::ofstream bodyOutStream = std::ofstream(bodyOutputFilePath, std::ios::out | std::ios::binary);
	bodyOutStream << vident0 << "// *******************************************************************************" << std::endl;
	bodyOutStream << vident0 << "//" << std::endl;
	bodyOutStream << vident0 << "// > OpenGL Function Variables" << std::endl;
	bodyOutStream << vident0 << "//" << std::endl;
	bodyOutStream << vident0 << "// Automatically generated. Do not modify by hand!" << std::endl;
	bodyOutStream << vident0 << "//" << std::endl;
	bodyOutStream << vident0 << "// *******************************************************************************" << std::endl;
	bodyOutStream << std::endl;
	for(auto it : prototypes) {
		const std::string version = Trim(it.first);
		const std::vector<GLPrototype> prototypeList = it.second;

		std::string ident0 = GetTabbedString(VariablesIdent);
		std::string ident1 = GetTabbedString(VariablesIdent + 1);

		bodyOutStream << "#" << ident0 << "if " << version << std::endl;

		for(auto prototype : prototypeList) {
			std::string originalName = Trim(prototype.name);
			std::string typedefName = MakeFunctionPtrName(FunctionPrefix, originalName);
			std::string staticFunctionName = FunctionPrefix + originalName;
			bodyOutStream << ident1 << ApiName << " " << typedefName << "* " << staticFunctionName << " = " << NullName << ";" << std::endl;
		}

		bodyOutStream << "#" << ident0 << "endif //" << version << std::endl;
		bodyOutStream << std::endl;
	}

	std::string bident0 = GetTabbedString(LoaderIdent);
	bodyOutStream << bident0 << "// *******************************************************************************" << std::endl;
	bodyOutStream << bident0 << "//" << std::endl;
	bodyOutStream << bident0 << "// > OpenGL Function Loader" << std::endl;
	bodyOutStream << bident0 << "//" << std::endl;
	bodyOutStream << bident0 << "// Automatically generated. Do not modify by hand!" << std::endl;
	bodyOutStream << bident0 << "//" << std::endl;
	bodyOutStream << bident0 << "// *******************************************************************************" << std::endl;
	for(auto it : prototypes) {
		const std::string version = Trim(it.first);
		const std::vector<GLPrototype> prototypeList = it.second;

		std::string ident0 = GetTabbedString(LoaderIdent);
		std::string ident1 = GetTabbedString(LoaderIdent + 1);

		bodyOutStream << "#" << ident0 << "if " << version << std::endl;

		for(auto prototype : prototypeList) {
			std::string originalName = Trim(prototype.name);
			std::string funcPtrName = MakeFunctionPtrName(FunctionPrefix, originalName);
			std::string staticFunctionName = FunctionPrefix + originalName;
			bodyOutStream << ident1 << staticFunctionName << " = (" << funcPtrName << " *)" << GetProcAddressName << "(state, \"" << originalName << "\");" << std::endl;
		}

		bodyOutStream << "#" << ident0 << "endif //" << version << std::endl;
		bodyOutStream << std::endl;
	}
	bodyOutStream.flush();
	bodyOutStream.close();

	return 0;
}