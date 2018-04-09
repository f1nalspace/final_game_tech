using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ProtParser
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        enum TokenType
        {
            None = 0,
            Char,
            Ident,
            BraceBegin,
            BraceEnd,
            ArgumentSeparator,
            Pointer,
        }

        class ParseInfo
        {
            public int LineNumber;
            public int CursorPos;

            public override string ToString()
            {
                return $"Line {LineNumber}, Cursor {CursorPos}";
            }
        }

        class Token
        {
            public TokenType Type;
            public char CharValue;
            public string IdentValue;
            public ParseInfo Info;

            public string GetValue()
            {
                switch (Type)
                {
                    case TokenType.Ident:
                        return IdentValue;
                    default:
                        return "" + CharValue;
                }
            }

            public override string ToString()
            {
                return $"{GetValue()} [{Info}]";
            }
        }

        class TokenizerState
        {
            public string Buffer;
            public int BufferPos;
            public int BufferLen;
            public int LineIndex;
            public int CursorStartForLine;

            public TokenizerState(string buffer)
            {
                Buffer = buffer;
                BufferPos = 0;
                BufferLen = buffer.Length;
                LineIndex = 0;
                CursorStartForLine = 0;
            }

            public bool IsEndOfStream()
            {
                bool result = BufferPos >= BufferLen;
                return (result);
            }

            public bool NextChar()
            {
                if (BufferPos < BufferLen)
                {
                    ++BufferPos;
                    return true;
                }
                else
                    return false;
            }

            public char GetChar()
            {
                if (BufferPos < BufferLen)
                    return Buffer[BufferPos];
                else
                    return '\0';
            }

            public void NextLine()
            {
                ++LineIndex;
                CursorStartForLine = BufferPos + 1;
            }

            public ParseInfo CreateInfo()
            {
                ParseInfo result = new ParseInfo()
                {
                    CursorPos = CursorStartForLine + 1,
                    LineNumber = LineIndex + 1,
                };
                return (result);
            }
        }

        class TokenizerOutput
        {
            public readonly List<Token> Tokens;

            public TokenizerOutput()
            {
                Tokens = new List<Token>();
            }

            public void AddSymbol(TokenType type, char c, ParseInfo info)
            {
                Token newTok = new Token()
                {
                    Type = type,
                    CharValue = c,
                    Info = info,
                };
                Tokens.Add(newTok);
            }

            public void AddChar(char c, ParseInfo info)
            {
                Token newTok = new Token()
                {
                    Type = TokenType.Char,
                    CharValue = c,
                    Info = info,
                };
                Tokens.Add(newTok);
            }

            public void AddIdent(string ident, ParseInfo info)
            {
                Token newTok = new Token()
                {
                    Type = TokenType.Ident,
                    IdentValue = ident,
                    Info = info,
                };
                Tokens.Add(newTok);
            }
        }

        private List<Token> Tokenize(string buffer)
        {
            TokenizerOutput ctx = new TokenizerOutput();
            TokenizerState state = new TokenizerState(buffer);
            while (!state.IsEndOfStream())
            {
                while (!state.IsEndOfStream() && state.GetChar() != '\n' && char.IsWhiteSpace(state.GetChar()))
                    state.NextChar();
                if (state.IsEndOfStream()) break;
                char c = state.GetChar();
                switch (c)
                {
                    case '\n':
                        state.NextLine();
                        state.NextChar();
                        break;
                    case '(':
                        ctx.AddSymbol(TokenType.BraceBegin, c, state.CreateInfo());
                        state.NextChar();
                        break;
                    case ')':
                        ctx.AddSymbol(TokenType.BraceEnd, c, state.CreateInfo());
                        state.NextChar();
                        break;
                    case ',':
                        ctx.AddSymbol(TokenType.ArgumentSeparator, c, state.CreateInfo());
                        state.NextChar();
                        break;
                    case '*':
                        ctx.AddSymbol(TokenType.Pointer, c, state.CreateInfo());
                        state.NextChar();
                        break;
                    default:
                        if (char.IsLetter(c) || c == '_')
                        {
                            int start = state.BufferPos;
                            while (!state.IsEndOfStream() && (char.IsLetterOrDigit(state.GetChar()) || (state.GetChar() == '_')))
                                state.NextChar();
                            int len = state.BufferPos - start;
                            string ident = buffer.Substring(start, len);
                            ctx.AddIdent(ident, state.CreateInfo());
                        }
                        else
                        {
                            ctx.AddChar(c, state.CreateInfo());
                            state.NextChar();
                        }
                        break;
                }
            }
            return (ctx.Tokens);
        }

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

        string PrintNames(IList<string> names)
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

        private string ParseFunctionPrototypes(string source)
        {
            StringBuilder s = new StringBuilder();

            try
            {

                List<FunctionPrototype> funcs = new List<FunctionPrototype>();

                if (!string.IsNullOrEmpty(source))
                {
                    List<Token> tokens = Tokenize(source);

                    List<Token> leftCache = new List<Token>();
                    List<Token> argumentsCache = new List<Token>();
                    ParseState state = ParseState.FunctionStart;

                    FunctionPrototype func = null;

                    int tokenIndex = 0;
                    while (tokenIndex < tokens.Count)
                    {
                        Token token = tokens[tokenIndex];
                        switch (state)
                        {
                            case ParseState.FunctionStart:
                                {
                                    if (token.Type == TokenType.BraceBegin)
                                    {
                                        if (leftCache.Count == 0)
                                            throw new Exception($"No tokens before '{token}'!");
                                        Token nameToken = leftCache.Last();
                                        if (nameToken.Type != TokenType.Ident)
                                            throw new Exception($"Expected token type '{TokenType.Ident}' but got token '{token}'!");
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
                                        if (token.Type == TokenType.BraceEnd)
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
                                        else if (token.Type == TokenType.ArgumentSeparator)
                                        {
                                            ++tokenIndex;

                                            FunctionArgumentPrototype newArg = new FunctionArgumentPrototype();
                                            newArg.Names.AddRange(argNames);
                                            func.Args.Add(newArg);
                                            argNames.Clear();

                                            if (tokenIndex == tokens.Count)
                                                throw new IndexOutOfRangeException($"Missing token after argument separator ',' on token '{token}'");
                                            token = tokens[tokenIndex];
                                            if (token.Type != TokenType.Ident)
                                                throw new Exception($"Expected token type '{TokenType.Ident}' but got token '{token}'!");
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

#if false
                int beginIdx = tokens.FindIndex((t) => t.Type == TokenType.BraceBegin);
                int endIdx = tokens.FindLastIndex((t) => t.Type == TokenType.BraceEnd);
                if (beginIdx > -1 && endIdx > -1)
                {
                    List<Token> beforeTokens = new List<Token>();
                    List<Token> funcTokens = new List<Token>();
                    for (int i = 0; i < beginIdx; ++i)
                    {
                        Token token = tokens[i];
                        beforeTokens.Add(token);
                    }
                    for (int i = beginIdx + 1; i < endIdx; ++i)
                    {
                        Token token = tokens[i];
                        funcTokens.Add(token);
                    }
                    if (beforeTokens.Count > 0 && beforeTokens.Last().Type == TokenType.Ident)
                    {
                        string funcName = beforeTokens.Last().IdentValue;
                        FunctionPrototype func = new FunctionPrototype()
                        {
                            Name = funcName,
                        };
                        for (int i = 0; i < beforeTokens.Count - 1; ++i)
                        {
                            Token token = beforeTokens[i];
                            func.Returns.Add(token.ToString());
                        }

                        FunctionArgumentPrototype newArg = new FunctionArgumentPrototype();
                        foreach (var argToken in funcTokens)
                        {
                            if (argToken.Type == TokenType.ArgumentSeparator)
                            {
                                func.Args.Add(newArg);
                                newArg = new FunctionArgumentPrototype();
                            }
                            else
                                newArg.Names.Add(argToken.ToString());
                        }
                        if (newArg.Names.Count > 0)
                            func.Args.Add(newArg);

                        funcs.Add(func);
                    }
                    else
                        throw new Exception($"Name ident for function missing on token '{token}'!");
                }
#endif
                }

                if (funcs.Count > 0)
                {

                    string prefix = tbPrefix.Text;

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

                    string loadMacro = tbLoadMacro.Text;
                    string loadLibHandle = tbLoadLibHandle.Text;
                    string loadLibName = tbLoadLibName.Text;
                    string loadLibFieldPrefix = tbLoadLibFieldPrefix.Text;

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

        private void UpdateTarget()
        {
            string parsed = ParseFunctionPrototypes(tbSource.Text);
            tbTarget.Text = parsed;
        }

        private void tbSource_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibHandle_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibName_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibFieldPrefix_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbPrefix_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadMacro_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }
    }
}
