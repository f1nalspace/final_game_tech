using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace FPLDocumentationCreator
{
    class Program
    {
        static string ParseComment(ref StringReader charReader)
        {
            StringBuilder result = new StringBuilder();
            char curChar = char.MinValue;
            while ((curChar = (char)charReader.Peek()) != char.MaxValue)
            {
                charReader.Read();
                if (curChar == '*')
                {
                    char nextChar = (char)charReader.Peek();
                    if (nextChar == '/')
                    {
                        charReader.Read();
                        break;
                    }
                    else
                    {
                        result.Append(curChar);
                    }
                }
                else
                {
                    result.Append(curChar);
                }
            }
            return result.ToString();
        }

        static string ParseIdent(ref StringReader charReader, char firstChar)
        {
            StringBuilder result = new StringBuilder();
            result.Append(firstChar);
            char curChar = char.MinValue;
            while ((curChar = (char)charReader.Peek()) != char.MaxValue)
            {
                if (char.IsLetterOrDigit(curChar) || (curChar == '_'))
                {
                    charReader.Read();
                    result.Append(curChar);
                }
                else
                    break;
            }
            return result.ToString();
        }

        static string ParseString(ref StringReader charReader)
        {
            StringBuilder result = new StringBuilder();
            char curChar = char.MinValue;
            bool escape = false;
            while ((curChar = (char)charReader.Peek()) != char.MaxValue)
            {
                charReader.Read();
                if (!escape)
                {
                    if (curChar == '"')
                        break;
                    else if (curChar == '\\')
                    {
                        escape = true;
                    }
                    else
                    {
                        result.Append(curChar);
                    }
                }
                else
                {
                    escape = false;
                }
            }
            return result.ToString();
        }

        static string ParseNumber(ref StringReader charReader, char firstChar)
        {
            StringBuilder result = new StringBuilder();
            result.Append(firstChar);
            char curChar = char.MinValue;
            bool hasDecimalSeparator = false;
            bool isHex = false;
            char peekedChar = (char)charReader.Peek();
            if (char.ToUpper(peekedChar) == (char)'X')
            {
                charReader.Read();
                result.Append(peekedChar);
                isHex = true;
            }
            while ((curChar = (char)charReader.Peek()) != char.MaxValue)
            {
                if (curChar == '.' && !hasDecimalSeparator)
                {
                    charReader.Read();
                    result.Append(curChar);
                    hasDecimalSeparator = true;
                }
                if ((char.IsNumber(curChar) || (isHex && (char.ToUpper(curChar) >= 'A') && char.ToUpper(curChar) <= 'F')))
                {
                    charReader.Read();
                    result.Append(curChar);
                }
                else
                    break;
            }
            return result.ToString();
        }

        static StringBuilder GenerateDocumentation(string filePath)
        {
            StringBuilder s = new StringBuilder();

            List<string> sourceLines = new List<string>();
            using (StreamReader reader = new StreamReader(filePath, Encoding.ASCII, false))
            {
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    sourceLines.Add(line);
                }
            }

            int referenceStartLineIndex = -1;
            int referenceEndLineIndex = -1;
            for (int i = 0; i < sourceLines.Count; i++)
            {
                string line = sourceLines[i];
                if (line.Contains("@REFERENCE_START"))
                    referenceStartLineIndex = i;
                if (line.Contains("@REFERENCE_END"))
                    referenceEndLineIndex = i;
            }

            StringBuilder referenceSource = new StringBuilder();
            List<string> referenceSourceLines = new List<string>();
            if (referenceStartLineIndex > -1 && referenceEndLineIndex > -1)
            {
                for (int i = referenceStartLineIndex; i < referenceEndLineIndex; i++)
                    referenceSource.AppendLine(sourceLines[i]);
            }

            StringReader charReader = new StringReader(referenceSource.ToString());
            char curChar = char.MinValue;
            StringBuilder buffer = new StringBuilder();
            string lastComment = null;
            string lastIdent = null;
            string lastString = null;
            char lastChar = char.MaxValue;
            string lastNumber = null;
            int lastCommentLine = -1;
            int lineNum = 1;
            while ((curChar = (char)charReader.Read()) != char.MaxValue)
            {
                if (curChar == '\n')
                    ++lineNum;
                if (char.IsWhiteSpace(curChar)) continue;
                switch (curChar)
                {
                    case '/':
                        {
                            char next = (char)charReader.Peek();
                            if (next == '/')
                            {
                                charReader.Read();
                                charReader.ReadLine();
                                ++lineNum;
                            }
                            else if (next == '*')
                            {
                                charReader.Read();
                                lastCommentLine = lineNum;
                                lastComment = ParseComment(ref charReader);
                                s.AppendLine($"Comment: {lastComment}");
                            }
                        }
                        break;
                    case '"':
                        {
                            lastString = ParseString(ref charReader);
                            s.AppendLine($"String: {lastString}");
                        }
                        break;
                    case '\'':
                        {
                            lastChar = curChar;
                            s.AppendLine($"Char: {lastChar}");
                        }
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        {
                            lastNumber = ParseNumber(ref charReader, curChar);
                            s.AppendLine($"Number: {lastNumber}");
                        }
                        break;
                    case ';':
                    case '(':
                    case ')':
                    case '{':
                    case '}':
                    case '=':
                    case ':':
                    case ',':
                    case '*':
                        s.AppendLine($"Character: {curChar}");
                        break;

                    default:
                        if (char.IsLetter(curChar))
                        {
                            lastIdent = ParseIdent(ref charReader, curChar);
                            s.AppendLine($"Ident: {lastIdent}");
                        }
                        break;
                }
            }

            return (s);
        }

        static void Main(string[] args)
        {
            try
            {
                if (args.Length < 1)
                    throw new ArgumentException("Source file argument is missing!");
                string sourceFilePath = args[0];
                if (!File.Exists(sourceFilePath))
                    throw new FileNotFoundException($"Source file '{sourceFilePath}' could not be found!");

                StringBuilder sourceCode = GenerateDocumentation(sourceFilePath);

                string outFilePath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), "Downloads", Path.GetFileName(sourceFilePath));
                using (StreamWriter writer = new StreamWriter(outFilePath, false, Encoding.ASCII))
                {
                    writer.Write(sourceCode.ToString());
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.ToString());
            }
        }
    }
}
