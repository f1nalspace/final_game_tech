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
            ParenOpen,
            ParenClose,
            Semicolon,
            ArgumentSeparator,
            Pointer,
            SingleLineComment,
            MultiLineComment,
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
            public TokenType Type { get; }
            public string Value { get; }
            public ParseInfo Info { get; }

            public Token(TokenType type, string value, ParseInfo info)
            {
                Type = type;
                Value = value ?? string.Empty;
                Info = info;
            }

            public override string ToString()
            {
                return $"{Value} [{Info}]";
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

            public bool Advance(int offset = 1)
            {
                if (BufferPos + offset <= BufferLen)
                {
                    BufferPos += offset;
                    return true;
                }
                else
                    return false;
            }

            public bool IsLineBreak(out int count)
            {
                char c = GetChar();
                char next = GetChar(1);
                if (c == '\r')
                {
                    if (next == '\n')
                    {
                        count = 2;
                        return true;
                    }
                    count = 0;
                    return false;
                }
                else if (c == '\n')
                {
                    count = 1;
                    return true;
                }
                count = 0;
                return false;
            }

            public void SkipLineBreak()
            {
                char c = GetChar();
                char nextChar = GetChar(1);
                if (c == '\r')
                {
                    Advance();
                    if (nextChar == '\n')
                    {
                        NextLine();
                        Advance();
                    }
                }
                else if (c == '\n')
                {
                    NextLine();
                    Advance();
                }
            }

            public char GetChar(int offset = 0)
            {
                if (BufferPos + offset < BufferLen)
                    return Buffer[BufferPos + offset];
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
                Token newTok = new Token(type, c.ToString(), info);
                Tokens.Add(newTok);
            }

            public void AddChar(char c, ParseInfo info)
            {
                Token newTok = new Token(TokenType.Char, c.ToString(), info);
                Tokens.Add(newTok);
            }

            public void AddIdent(string ident, ParseInfo info)
            {
                Token newTok = new Token(TokenType.Ident, ident, info);
                Tokens.Add(newTok);
            }

            public void AddSingleLineComment(string comment, ParseInfo info)
            {
                Token newTok = new Token(TokenType.SingleLineComment, comment, info);
                Tokens.Add(newTok);
            }

            public void AddMultiLineComment(string comment, ParseInfo info)
            {
                Token newTok = new Token(TokenType.MultiLineComment, comment, info);
                Tokens.Add(newTok);
            }
        }

        private static void SkipWhitespacesButKeepLineBreak(TokenizerState state)
        {
            while (!state.IsEndOfStream())
            {
                char c = state.GetChar();
                if (state.IsLineBreak(out _) || !char.IsWhiteSpace(c))
                    break;
                state.Advance();
            }
        }

        public static List<Token> Tokenize(string buffer)
        {
            TokenizerOutput ctx = new TokenizerOutput();
            TokenizerState state = new TokenizerState(buffer);
            while (!state.IsEndOfStream())
            {
                SkipWhitespacesButKeepLineBreak(state);
                if (state.IsEndOfStream())
                    break;
                char c = state.GetChar();
                switch (c)
                {
                    case '\r':
                    case '\n':
                        state.SkipLineBreak();
                        break;
                    case '(':
                        ctx.AddSymbol(TokenType.ParenOpen, c, state.CreateInfo());
                        state.Advance();
                        break;
                    case ')':
                        ctx.AddSymbol(TokenType.ParenClose, c, state.CreateInfo());
                        state.Advance();
                        break;
                    case ',':
                        ctx.AddSymbol(TokenType.ArgumentSeparator, c, state.CreateInfo());
                        state.Advance();
                        break;
                    case '*':
                        ctx.AddSymbol(TokenType.Pointer, c, state.CreateInfo());
                        state.Advance();
                        break;
                    case ';':
                        ctx.AddSymbol(TokenType.Semicolon, c, state.CreateInfo());
                        state.Advance();
                        break;

                    case '/':
                    {
                        char nextChar = state.GetChar(1);
                        if (nextChar == '/')
                        {
                            int start = state.BufferPos;
                            state.Advance(2);
                            while (!state.IsEndOfStream() && !state.IsLineBreak(out _))
                                state.Advance();
                            int len = state.BufferPos - start;
                            string comment = buffer.Substring(start, len);
                            ctx.AddSingleLineComment(comment, state.CreateInfo());
                        }
                        else if (nextChar == '*')
                        {
                            int start = state.BufferPos;
                            state.Advance(2);
                            while (!state.IsEndOfStream())
                            {
                                if (state.IsLineBreak(out _))
                                {
                                    state.SkipLineBreak();
                                    continue;
                                }

                                char n0 = state.GetChar();
                                char n1 = state.GetChar(1);
                                if (n0 == '*')
                                {
                                    if (n1 == '/')
                                    {
                                        state.Advance(2);
                                        break;
                                    }
                                    else
                                        state.Advance();
                                }
                                else
                                    state.Advance();
                            }
                            int len = state.BufferPos - start;
                            string comment = buffer.Substring(start, len);
                            ctx.AddMultiLineComment(comment, state.CreateInfo());
                        }
                        else
                        {
                            ctx.AddSymbol(TokenType.Char, c, state.CreateInfo());
                            state.Advance();
                        }
                    }
                    break;

                    default:
                        if (char.IsLetter(c) || c == '_')
                        {
                            int start = state.BufferPos;
                            while (!state.IsEndOfStream() && (char.IsLetterOrDigit(state.GetChar()) || (state.GetChar() == '_')))
                                state.Advance();
                            int len = state.BufferPos - start;
                            string ident = buffer.Substring(start, len);
                            ctx.AddIdent(ident, state.CreateInfo());
                        }
                        else
                        {
                            ctx.AddChar(c, state.CreateInfo());
                            state.Advance();
                        }
                        break;
                }
            }
            return (ctx.Tokens);
        }
    }
}
