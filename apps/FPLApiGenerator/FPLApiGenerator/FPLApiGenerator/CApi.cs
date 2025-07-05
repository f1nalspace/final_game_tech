using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.ComponentModel.DataAnnotations;
using System.Drawing;
using System.Linq;
using System.Text.RegularExpressions;

namespace FPLApiGenerator
{
    enum CApiType
    {
        Fixed = 0,
        Dynamic,
    }

    class CApi
    {
        public const string PublicTypePrefix = "fpl";
        public const string PublicDefinePrefix = "FPL_";
        public const string InternalDefinePrefix = "FPL__";
        public const string InternalFunctionPrefix = "fpl__";
        public const string InternalApiCall = "fpl_internal";
        public const string GlobalVariable = "fpl_globalvar";
        public const string FuncPrefix = "func_";
        public const string FuncPostfix = "";

        public string SystemName { get; private set; }
        public CApiType Type { get; }
        public IEnumerable<CFunction> Functions => _functions;
        public IEnumerable<CImplementation> Implementations => _implementations;

        public CType ContextType { get; }
        public CArgument ContextArgument { get; }

        public CType BackendType { get; }
        public CArgument BackendArgument { get; }

        public CType CommonType { get; }

        private readonly List<CFunction> _functions = new List<CFunction>();

        private readonly List<CImplementation> _implementations = new List<CImplementation>();

        public static readonly Regex NameRex = new Regex("[A-Z][a-zA-Z0-9_]+", RegexOptions.Compiled);

        public CApi(string systemName, CApiType type)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(systemName);

            if (!NameRex.IsMatch(systemName))
                throw new ArgumentException($"System name '{systemName}' argument does not match regex '{NameRex}'");

            SystemName = systemName.ToPascalCase();
            Type = type;

            string contextTypeName = $"{PublicTypePrefix}{SystemName}Context";
            string backendTypeName = $"{PublicTypePrefix}{SystemName}Backend";
            string commonTypeName = $"{PublicTypePrefix}Common{SystemName}";

            ContextType = new CType(contextTypeName, CTypeFlags.Pointer | CTypeFlags.OpaqueStruct);
            ContextArgument = new CArgument(ContextType, "context", CArgumentFlags.Builtin);

            BackendType = new CType(backendTypeName, CTypeFlags.Pointer | CTypeFlags.OpaqueStruct);
            BackendArgument = new CArgument(BackendType, "backend", CArgumentFlags.Builtin);

            CommonType = new CType(commonTypeName);
        }

        public void Rename(string systemName)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(systemName);

            if (!NameRex.IsMatch(systemName))
                throw new ArgumentException($"System name '{systemName}' argument does not match regex '{NameRex}'");

            SystemName = systemName.ToPascalCase();

            string contextTypeName = $"{PublicTypePrefix}{SystemName}Context";
            string backendTypeName = $"{PublicTypePrefix}{SystemName}Backend";
            string commonTypeName = $"{PublicTypePrefix}Common{SystemName}";

            ContextType.Name = contextTypeName;
            BackendType.Name = backendTypeName;
            CommonType.Name = commonTypeName;
        }

        public void AddFunctions(IEnumerable<CFunction> functions)
        {
            ArgumentNullException.ThrowIfNull(functions);
            _functions.AddRange(functions);
        }

        public void SetFunctions(IEnumerable<CFunction> functions)
        {
            ArgumentNullException.ThrowIfNull(functions);
            _functions.Clear();
            _functions.AddRange(functions);
        }

        public void AddImplementations(IEnumerable<CImplementation> implementations)
        {
            ArgumentNullException.ThrowIfNull(implementations);
            _implementations.AddRange(implementations);
        }

        public void SetImplementations(IEnumerable<CImplementation> implementations)
        {
            ArgumentNullException.ThrowIfNull(implementations);
            _implementations.Clear();
            _implementations.AddRange(implementations);
        }

        public void SetImplementations(params CImplementation[] implementations)
        {
            ArgumentNullException.ThrowIfNull(implementations);
            _implementations.Clear();
            _implementations.AddRange(implementations);
        }
    }
}
