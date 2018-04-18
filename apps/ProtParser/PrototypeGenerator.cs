using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProtParser
{
    class PrototypeGenerator
    {
        class FunctionArgumentPrototype
        {
            public readonly List<string> Names;

            public FunctionArgumentPrototype()
            {
                Names = new List<string>();
            }
        }

        class FunctionPrototype
        {
            public string Name;
            public readonly List<string> Returns;
            public readonly List<FunctionArgumentPrototype> Args;

            public FunctionPrototype()
            {
                Args = new List<FunctionArgumentPrototype>();
                Returns = new List<string>();
            }
        }

        static string PrintNames(IList<string> names)
        {
            StringBuilder s = new StringBuilder();
            for (int retIndex = 0; retIndex < names.Count; ++retIndex)
            {
                string retValue = names[retIndex];
                if (retIndex > 0)
                {
                    string prev = names[retIndex - 1];
                    if ("*".Equals(retValue))
                    {
                        if (!"*".Equals(prev))
                            s.Append(" ");
                    }
                    else
                    {
                        if (!"*".Equals(prev))
                            s.Append(" ");
                    }
                }
                s.Append(retValue);
            }
            return s.ToString();
        }


        enum ParseState
        {
            FunctionStart,
            FunctionArgs,
        }

        public static string ParseFunctionPrototypes(string source, Preset preset)
        {
            StringBuilder s = new StringBuilder();

            string prefix = preset.GetProperty("Prefix");
            string loadMacro = preset.GetProperty("LoadMacro");
            string loadLibHandle = preset.GetProperty("LoadLibHandle");
            string loadLibName = preset.GetProperty("LoadLibName");
            string loadLibFieldPrefix = preset.GetProperty("LoadLibFieldPrefix");

            try
            {

                List<FunctionPrototype> funcs = new List<FunctionPrototype>();

                if (!string.IsNullOrEmpty(source))
                {
                    List<FunctionTokenizer.Token> tokens = FunctionTokenizer.Tokenize(source);

                    List<FunctionTokenizer.Token> leftCache = new List<FunctionTokenizer.Token>();
                    List<FunctionTokenizer.Token> argumentsCache = new List<FunctionTokenizer.Token>();
                    ParseState state = ParseState.FunctionStart;

                    FunctionPrototype func = null;

                    int tokenIndex = 0;
                    while (tokenIndex < tokens.Count)
                    {
                        FunctionTokenizer.Token token = tokens[tokenIndex];
                        switch (state)
                        {
                            case ParseState.FunctionStart:
                                {
                                    if (token.Type == FunctionTokenizer.TokenType.BraceBegin)
                                    {
                                        if (leftCache.Count == 0)
                                            throw new Exception($"No tokens before '{token}'!");
                                        FunctionTokenizer.Token nameToken = leftCache.Last();
                                        if (nameToken.Type != FunctionTokenizer.TokenType.Ident)
                                            throw new Exception($"Expected token type '{FunctionTokenizer.TokenType.Ident}' but got token '{token}'!");
                                        string funcName = nameToken.GetValue();
                                        leftCache.RemoveAt(leftCache.Count - 1);
                                        func = new FunctionPrototype()
                                        {
                                            Name = funcName,
                                        };
                                        funcs.Add(func);
                                        foreach (var returnTok in leftCache)
                                            func.Returns.Add(returnTok.GetValue());
                                        leftCache.Clear();
                                        ++tokenIndex;
                                        state = ParseState.FunctionArgs;
                                    }
                                    else
                                    {
                                        leftCache.Add(token);
                                        ++tokenIndex;
                                    }
                                }
                                break;
                            case ParseState.FunctionArgs:
                                {
                                    Debug.Assert(leftCache.Count == 0);
                                    Debug.Assert(func != null);
                                    List<string> argNames = new List<string>();
                                    while (tokenIndex < tokens.Count)
                                    {
                                        token = tokens[tokenIndex];
                                        if (token.Type == FunctionTokenizer.TokenType.BraceEnd)
                                        {
                                            ++tokenIndex;

                                            FunctionArgumentPrototype newArg = new FunctionArgumentPrototype();
                                            newArg.Names.AddRange(argNames);
                                            func.Args.Add(newArg);

                                            // Done with function
                                            func = null;
                                            state = ParseState.FunctionStart;
                                            break;
                                        }
                                        else if (token.Type == FunctionTokenizer.TokenType.ArgumentSeparator)
                                        {
                                            ++tokenIndex;

                                            FunctionArgumentPrototype newArg = new FunctionArgumentPrototype();
                                            newArg.Names.AddRange(argNames);
                                            func.Args.Add(newArg);
                                            argNames.Clear();

                                            if (tokenIndex == tokens.Count)
                                                throw new IndexOutOfRangeException($"Missing token after argument separator ',' on token '{token}'");
                                            token = tokens[tokenIndex];
                                            if (token.Type != FunctionTokenizer.TokenType.Ident)
                                                throw new Exception($"Expected token type '{FunctionTokenizer.TokenType.Ident}' but got token '{token}'!");
                                        }
                                        else
                                        {
                                            argNames.Add(token.GetValue());
                                            ++tokenIndex;
                                        }
                                    }
                                }
                                break;
                            default:
                                throw new Exception($"Unsupported parse state '{state}' on token '{token}'");
                        }
                    }
                }

                if (funcs.Count > 0)
                {
                    
                    s.AppendLine("// Prototypes");
                    Dictionary<FunctionPrototype, string> funcToTypeNameMap = new Dictionary<FunctionPrototype, string>();
                    Dictionary<FunctionPrototype, string> funcToNameMap = new Dictionary<FunctionPrototype, string>();
                    foreach (var func in funcs)
                    {
                        funcToTypeNameMap.Add(func, $"{prefix.ToLower()}{func.Name}");
                        funcToNameMap.Add(func, func.Name);

                        s.Append($"#define {prefix}{func.Name}(name) ");

                        string returnStr = PrintNames(func.Returns);
                        s.Append(returnStr);
                        if (s[s.Length - 1] != '*')
                            s.Append(" ");
                        s.Append("name(");
                        for (int argIndex = 0; argIndex < func.Args.Count; ++argIndex)
                        {
                            if (argIndex > 0)
                                s.Append(", ");
                            string argStr = PrintNames(func.Args[argIndex].Names);
                            s.Append(argStr);
                        }
                        s.Append(")");
                        s.AppendLine();

                        s.Append($"typedef {prefix}{func.Name}({prefix.ToLower()}{func.Name});");
                        s.AppendLine();
                    }

                    s.AppendLine();
                    s.AppendLine("// Declarations");

                    foreach (var func in funcs)
                    {
                        string n = funcToTypeNameMap[func];
                        s.AppendLine($"{n} *{func.Name};");
                    }

                    s.AppendLine();
                    s.AppendLine("// Load");

                    foreach (var func in funcs)
                    {
                        string funcName = funcToNameMap[func];
                        string typeName = funcToTypeNameMap[func];
                        s.AppendLine($"{loadMacro}({loadLibHandle}, {loadLibName}, {loadLibFieldPrefix}{funcName}, {typeName}, \"{funcName}\");");
                    }
                }
            }
            catch (Exception e)
            {
                s.AppendLine($"Error: {e.Message}");
            }

            return (s.ToString());
        }
    }
}
