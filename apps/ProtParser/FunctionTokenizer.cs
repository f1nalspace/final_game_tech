using System;
using System.Collections.Generic;

namespace ProtParser
{
    class FunctionTokenizer
    {
        public enum TokenType
        {
            None = 0,
            Char,
            Ident,
            BraceBegin,
            BraceEnd,
            ArgumentSeparator,
            Pointer,
        }

        public class ParseInfo
        {
            public int LineNumber;
            public int CursorPos;

            public override string ToString()
            {
                return $"Line {LineNumber}, Cursor {CursorPos}";
            }
        }

        public class Token
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

        public class TokenizerState
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

        public class TokenizerOutput
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

        public static List<Token> Tokenize(string buffer)
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
    }
}
