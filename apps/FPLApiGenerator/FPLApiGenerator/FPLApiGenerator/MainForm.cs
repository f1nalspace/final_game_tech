using System;
using System.Collections.Generic;
using System.Reflection.Metadata;
using System.Text;
using System.Windows.Forms;

namespace FPLApiGenerator
{
    public partial class MainForm : Form
    {
        private CApi _api;

        public MainForm()
        {
            InitializeComponent();

            _api = CreateDefaultApi("GameController", CApiType.Dynamic);

            RefreshApi(_api);
            Generate();
        }

        private static CApi ConvertApiType(CApi source, CApiType type)
        {
            ArgumentNullException.ThrowIfNull(source);

            CApi newApi = CreateDefaultApi(source.SystemName, type);

            // TODO: Migrate functions

            newApi.SetImplementations(source.Implementations);

            return newApi;
        }

        private static CApi CreateDefaultApi(string systemName, CApiType type)
        {
            CApi api = new CApi(systemName, type);

            CArgument contextArgument = api.ContextArgument;

            CArgument backendArgument = api.BackendArgument;

            List<CFunction> functions = new List<CFunction>();
            if (type == CApiType.Dynamic)
            {
                functions.Add(new CFunction(CType.BoolType, "Initialize", contextArgument, backendArgument));
                functions.Add(new CFunction(CType.VoidType, "Release", contextArgument, backendArgument));
            }
            api.SetFunctions(functions);

            api.SetImplementations(new CImplementation("Null", new EightCC("NULL")));

            return api;
        }

        private void Clear()
        {
            rtbOutput.Clear();
        }

        private static string CreateSeparator(char c, int count)
        {
            string s = new string(c, count);
            return s;
        }

        private void Add(ReadOnlySpan<char> input)
        {
            rtbOutput.AppendText(input.ToString());
        }

        private void AddLine(ReadOnlySpan<char> input)
        {
            if (input.Length > 0)
                Add(input);
            Add(Environment.NewLine);
        }

        private void AddLine()
        {
            Add(Environment.NewLine);
        }

        void GenerateDynamicApi(CApi api)
        {
            string definitionSeperator = CreateSeparator('*', 80);
            string implementationSeperator = CreateSeparator('#', 80);

            string systemName = api.SystemName;
            string systemNameCamelCase = systemName.ToCamelCase();

            string backendName = $"{systemName}Backend";

            string upperSystemName = systemName.ToUpper();

            string backendDefineName = $"{CApi.InternalDefinePrefix}{upperSystemName}_BACKEND";

            string backendDataPaddingDefineName = $"{backendDefineName}_DATA_PADDING";
            string backendDataOffsetDefineName = $"{backendDefineName}_DATA_OFFSET";
            string backendImplDefineName = $"{backendDefineName}_IMPL";

            string enableDefineName = $"{CApi.InternalDefinePrefix}ENABLE_{upperSystemName}";

            string funcPostfixUpper = CApi.FuncPostfix.ToUpper();
            string funcPrefixUpper = CApi.FuncPrefix.ToUpper();

            string initFlagsEnumName = $"fplInitFlags";

            string initFlagsValueName = $"{initFlagsEnumName}_{systemName}";

            string backendTypeEnumName = $"{CApi.PublicTypePrefix}{backendName}Type";

            string settingsName = $"{systemName}Settings";

            string settingsStructName = $"{CApi.PublicTypePrefix}{settingsName}";

            IndentState indentState = new IndentState();

            //
            // Public API
            //
            AddLine($"// {definitionSeperator}");
            AddLine($"//");
            AddLine($"// > Public API");
            AddLine($"//");
            AddLine($"// {definitionSeperator}");

            AddLine("// Only add the new init flags to the enum");
            AddLine($"typedef enum {initFlagsEnumName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{initFlagsEnumName}_None = 0,");
                AddLine($"{indent}{initFlagsValueName} = 1 << 31,");
            }
            AddLine($"}} {initFlagsEnumName};");
            AddLine();

            AddLine($"// {systemName} backend type");
            AddLine($"typedef enum {backendTypeEnumName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{backendTypeEnumName}_None = 0,");
                foreach (CImplementation impl in api.Implementations)
                    AddLine($"{indent}{backendTypeEnumName}_{impl.Name},");
            }
            AddLine($"}} {backendTypeEnumName};");
            AddLine();

            AddLine($"// {systemName} backend settings");
            AddLine($"typedef struct {settingsStructName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{backendTypeEnumName} type;");
            }
            AddLine($"}} {settingsStructName};");
            AddLine();


            //
            // Internal backend API
            //
            string apiImplementedDefineName = $"{backendDefineName}_API_IMPLEMENTED";
            AddLine($"// {definitionSeperator}");
            AddLine($"//");
            AddLine($"// > {backendDefineName} API");
            AddLine($"//");
            AddLine($"// {definitionSeperator}");
            AddLine($"#if !defined({apiImplementedDefineName}) && defined({enableDefineName})");
            AddLine($"#define {apiImplementedDefineName}");
            AddLine();

            AddLine($"//");
            AddLine($"// {systemName} backend forward declarations");
            AddLine($"//");
            AddLine();
            AddLine($"{api.ContextType};");
            AddLine($"{api.BackendType};");
            AddLine();

            AddLine($"//");
            AddLine($"// {systemName} backend function definitions");
            AddLine($"//");
            AddLine();

            Dictionary<CFunction, string> functionTableFieldNamesMap = new Dictionary<CFunction, string>();
            Dictionary<CFunction, string> functionTableDefineNamesMap = new Dictionary<CFunction, string>();

            foreach (CFunction function in api.Functions)
            {
                string functionName = function.Name;
                string upperFunctionName = functionName.ToUpper();

                CType resultType = function.ResultType;

                string functionDefineName = $"{CApi.InternalDefinePrefix}{funcPrefixUpper}{upperSystemName}_BACKEND_{upperFunctionName}{funcPostfixUpper}";
                string functionTypedefName = $"{CApi.InternalFunctionPrefix}{CApi.FuncPrefix}{systemName}_backend_{functionName}{funcPostfixUpper}";
                string functionTableFieldName = functionName.ToCamelCase();
                Add($"#define {functionDefineName}(name) {resultType} name(");

                if (function.Arguments.Length == 0)
                    Add("void");
                else
                {
                    StringBuilder args = new StringBuilder();
                    foreach (CArgument argument in function.Arguments)
                    {
                        if (args.Length > 0)
                            args.Append(", ");
                        args.Append(argument);
                    }
                    Add(args.ToString());
                }

                AddLine(")");

                AddLine($"typedef {functionDefineName}({functionTypedefName});");
                AddLine();

                functionTableFieldNamesMap.Add(function, functionTableFieldName);
                functionTableDefineNamesMap.Add(function, functionDefineName);
            }

            string tableStructName = $"{CApi.PublicTypePrefix}{backendName}Table";
            AddLine($"//");
            AddLine($"// {systemName} backend function table");
            AddLine($"//");
            AddLine();

            AddLine($"typedef struct {tableStructName} {{");
            using (var indent = new Indent(indentState))
            {
                foreach (CFunction function in api.Functions)
                {
                    string functionName = function.Name;
                    string funcTypeName = $"{CApi.InternalFunctionPrefix}{CApi.FuncPrefix}{systemName}_backend_{functionName}{funcPostfixUpper}";
                    string fieldName = functionTableFieldNamesMap[function];
                    AddLine($"{indent}{funcTypeName} *{fieldName};");
                }
            }
            AddLine($"}} {tableStructName};");
            AddLine();

            AddLine($"//");
            AddLine($"// {systemName} backend descriptor");
            AddLine($"//");
            AddLine();

            string idTypeName = $"{CApi.PublicTypePrefix}{backendName}Id";
            AddLine($"// 8-CC code of the {systemName} backend");
            AddLine($"typedef uint64_t {idTypeName};");
            AddLine();

            string idNameStructName = $"{CApi.PublicTypePrefix}{backendName}IdName";
            CArgument idnameNameArgument = new CArgument(CType.StringType, "name");
            AddLine($"typedef struct {idNameStructName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{idTypeName} id;");
                AddLine($"{indent}{idnameNameArgument};");
                AddLine($"}} {idNameStructName};");
            }
            AddLine();

            string headerStructName = $"{CApi.PublicTypePrefix}{backendName}Header";
            CArgument sizeArgument = new CArgument(CType.SizeType, "size");
            CArgument isValidArgument = new CArgument(CType.B32Type, "isValid");
            AddLine($"typedef struct {headerStructName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{idNameStructName} idName;");
                AddLine($"{indent}{backendTypeEnumName} type;");
                AddLine($"{indent}{sizeArgument};");
                AddLine($"{indent}{isValidArgument};");
            }
            AddLine($"}} {headerStructName};");
            AddLine();

            string descriptorStructName = $"{CApi.PublicTypePrefix}{backendName}Descriptor";
            AddLine($"typedef struct {descriptorStructName} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{headerStructName} header;");
                AddLine($"{indent}{tableStructName} table;");
            }
            AddLine($"}} {descriptorStructName};");
            AddLine();

            AddLine($"// Stores internal data for the {systemName} backend");
            AddLine($"typedef struct {api.BackendType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}// Unused to prevent compile errors;");
                AddLine($"{indent}int unused;");
            }
            AddLine($"}} {api.BackendType.Name};");
            AddLine();

            AddLine($"#define {backendDataPaddingDefineName} 16");
            AddLine($"#define {backendDataOffsetDefineName} (sizeof({api.BackendType.Name}) + {backendDataPaddingDefineName})");
            AddLine($"#define {backendImplDefineName}(backend, type) (type *)(((uint8_t *)(backend) + {backendDataOffsetDefineName}))");
            AddLine();

            AddLine($"#endif // {apiImplementedDefineName} && {enableDefineName}");

            AddLine();
            AddLine();
            AddLine();

            //
            // Backends implementations
            //
            string commonImplementedDefineName = $"{backendDefineName}S_IMPLEMENTED";
            AddLine($"// {definitionSeperator}");
            AddLine($"//");
            AddLine($"// > {backendDefineName}S");
            AddLine($"//");
            AddLine($"// {definitionSeperator}");
            AddLine($"#if !defined({commonImplementedDefineName}) && defined({enableDefineName})");
            AddLine($"#define {commonImplementedDefineName}");
            AddLine();

            AddLine($"// Stores common data for the {systemName} backend");
            AddLine($"typedef struct {api.ContextType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}// Unused to prevent compile errors;");
                AddLine($"{indent}int unused;");
            }
            AddLine($"}} {api.ContextType.Name};");
            AddLine();

            AddLine($"// Stores the table and all relevant pointers and data for the {systemName} backend");
            AddLine($"typedef struct {api.CommonType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}// Function table;");
                AddLine($"{indent}{tableStructName} table;");
                AddLine($"{indent}// Context;");
                AddLine($"{indent}{api.ContextType.Name} context;");
                AddLine($"{indent}// Reference to the backend;");
                AddLine($"{indent}{api.BackendType.Name} *backend;");
            }
            AddLine($"}} {api.CommonType.Name};");
            AddLine();

            foreach (CImplementation impl in api.Implementations)
            {
                EightCC code = impl.Code;

                string implUpperName = impl.Name.ToUpper();

                string enableImplDefineName = $"{enableDefineName}";

                string descriptorImplName = $"{backendName}{impl.Name}Descriptor";

                AddLine($"// {implementationSeperator}");
                AddLine("//");
                AddLine($"// > {backendDefineName}_{implUpperName}");
                AddLine("//");
                AddLine($"// {implementationSeperator}");
                AddLine();

                Dictionary<CFunction, string> functionImplementationNameMap = new Dictionary<CFunction, string>();

                foreach (CFunction function in api.Functions)
                {
                    string functionName = function.Name;
                    string upperFunctionName = functionName.ToUpper();
                    string functionDefineName = functionTableDefineNamesMap[function];

                    string functionImplementationName = $"{CApi.InternalFunctionPrefix}{systemName}_Backend_{impl.Name}_{functionName}";

                    AddLine($"//");
                    AddLine($"// {functionName} of the {impl.Name} backend");
                    AddLine($"//");
                    AddLine($"{CApi.InternalApiCall} {functionDefineName}({functionImplementationName}) {{");
                    using (var indent = new Indent(indentState))
                    {
                        if (function.ResultType == CType.VoidType)
                            AddLine();
                        else
                        {
                            string defaultValue = CValue.GetDefaultValue(function.ResultType);
                            AddLine($"{indent} return {defaultValue};");
                        }
                    }
                    AddLine($"}}");
                    AddLine();

                    functionImplementationNameMap.Add(function, functionImplementationName);
                }

                AddLine($"// Descriptor table of the {impl.Name} backend");
                AddLine($"{CApi.GlobalVariable} {descriptorStructName} {CApi.InternalFunctionPrefix}global_{descriptorImplName} = {{");
                using (var indent1 = new Indent(indentState))
                {
                    AddLine($"{indent1}fplStructField({descriptorStructName}, header, {{");
                    using (var indent2 = new Indent(indentState))
                    {
                        AddLine($"{indent2}fplStructField({headerStructName}, idName, {{");
                        using (var indent3 = new Indent(indentState))
                        {
                            AddLine($"{indent3}fplStructField({idNameStructName}, id, 0x{code.ToHex()}),");
                            AddLine($"{indent3}fplStructField({idNameStructName}, name, \"{impl.Name}\"),");
                        }
                        AddLine($"{indent2}}}),");
                        AddLine($"{indent2}fplStructField({headerStructName}, type, {backendTypeEnumName}_{impl.Name}),");
                        AddLine($"{indent2}fplStructField({headerStructName}, isValid, true),");
                    }
                    AddLine($"{indent1}}}),");
                    AddLine($"{indent1}fplStructField({descriptorStructName}, table, {{");
                    foreach (CFunction function in api.Functions)
                    {
                        string functionFieldName = functionTableFieldNamesMap[function];
                        string functionImplementationName = functionImplementationNameMap[function];
                        using (var indent2 = new Indent(indentState))
                        {
                            AddLine($"{indent2}fplStructField({tableStructName}, {functionFieldName}, {functionImplementationName}),");
                        }
                    }
                    AddLine($"{indent1}}}),");
                }
                AddLine($"}};");
                AddLine();
            }

            AddLine($"#endif // {commonImplementedDefineName} && {enableDefineName}");
            AddLine();

            //
            // Platform States
            //
            string platformStatesDefined = $"{CApi.InternalDefinePrefix}PLATFORM_STATES_DEFINED";
            AddLine($"// {implementationSeperator}");
            AddLine($"//");
            AddLine($"// > PLATFORM_STATES");
            AddLine($"//");
            AddLine($"// {implementationSeperator}");
            AddLine($"#if !defined({platformStatesDefined})");
            AddLine($"#define {platformStatesDefined}");
            AddLine();

            string platformMemoryBlockStruct = $"{CApi.InternalFunctionPrefix}fpl__PlatformMemoryBlock";
            AddLine("// Platform Memory Block (Do not change)");
            AddLine($"typedef struct {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{CType.SizeType} size;");
                AddLine($"{indent}{CType.UPtrType} offset;");
            }
            AddLine($"}} {platformMemoryBlockStruct};");
            AddLine();

            string backendStateStruct = $"{CApi.InternalFunctionPrefix}PlatformBackendState";
            AddLine("// Platform Backend State (Do not change)");
            AddLine($"typedef struct {{");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{CType.VoidType} *mem;");
                AddLine($"{indent}{CType.SizeType} *size;");
                AddLine($"{indent}{CType.SizeType} maxBackendSize;");
                AddLine($"{indent}{CType.SizeType} offsetToBackend;");
            }
            AddLine($"}} {backendStateStruct};");
            AddLine();

            string platformAppStateStruct = $"{CApi.InternalFunctionPrefix}PlatformAppState";
            AddLine("// Platform Application State");
            AddLine($"typedef struct {platformAppStateStruct} {platformAppStateStruct};");
            AddLine($"struct {platformAppStateStruct} {{");
            AddLine($"#if defined({enableDefineName})");
            using (var indent = new Indent(indentState))
            {
                AddLine($"{indent}{backendStateStruct} {systemNameCamelCase};");
            }
            AddLine($"#endif");
            AddLine($"}};");
            AddLine();

            string backendMemoryBlockVariableName = $"{systemNameCamelCase}MemoryBlock";
            AddLine($"fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {{");
            using (var indent1 = new Indent(indentState))
            {
                AddLine($"{indent1}//");
                AddLine($"{indent1}// Compute platform memory");
                AddLine($"{indent1}//");
                AddLine();
                AddLine($"{indent1}{platformMemoryBlockStruct} {backendMemoryBlockVariableName} = fplZeroInit;");
                AddLine();

                string settingsVariableName = $"{systemNameCamelCase}Settings";
                string maxBackendSizeVariableName = $"max{backendName}Size";
                string offsetToBackendVariableName = $"offsetTo{backendName}";
                AddLine($"{indent1}// Compute {systemName} backend memory");
                AddLine($"#\tif defined({enableDefineName})");
                AddLine($"{indent1}{CType.SizeType} {maxBackendSizeVariableName} = 0;");
                AddLine($"{indent1}{CType.SizeType} {offsetToBackendVariableName} = 0;");
                AddLine($"{indent1}if (fplIsMaskSet(initFlags, {initFlagsValueName})) {{");
                using (var indent2 = new Indent(indentState))
                {
                    AddLine($"{indent2}{settingsStructName} {settingsVariableName} = fplZeroInit;");
                    AddLine($"{indent2}if (initSettings != fpl_null) {{");
                    using (var indent3 = new Indent(indentState))
                    {
                        AddLine($"{indent3}{settingsVariableName} = initSettings->{systemNameCamelCase}");
                    }
                    AddLine($"{indent2}}} else {{");
                    using (var indent3 = new Indent(indentState))
                    {
                        AddLine($"{indent3}fplSetDefault{settingsName}(&{settingsVariableName});");
                    }
                    AddLine($"{indent2}}}");
                    AddLine($"{indent2}fpl__PushPlatformMemory(${backendMemoryBlockVariableName}, {maxBackendSizeVariableName}, 16, 0);");
                    AddLine($"{indent2}{offsetToBackendVariableName} = 0;");
                }
                AddLine($"{indent1}}}");
                AddLine($"#{indent1}endif // {enableDefineName}");
            }
            AddLine($"}}");

            AddLine($"#endif // {platformStatesDefined}");
        }

        void GenerateFixedApi(CApi api)
        {
        }

        private void Generate(CApi api)
        {
            if (api.Type == CApiType.Dynamic)
                GenerateDynamicApi(api);
            else
                GenerateFixedApi(api);
        }

        private void Generate(object sender, EventArgs args)
        {
            Clear();
            Generate(_api);
        }

        private void Generate() => Generate(this, new EventArgs());

        private ListViewItem AddFunctionItem(CFunction function)
        {
            ListViewItem item = lvFunctions.Items.Add(function.Name);
            item.Tag = function;
            StringBuilder args = new StringBuilder();
            foreach (CArgument arg in function.Arguments)
            {
                string argText = arg.Name;
                if (args.Length > 0)
                    args.Append(", ");
                args.Append(argText);
            }
            item.SubItems.Add(args.ToString());
            item.SubItems.Add(function.ResultType.ToString());
            item.ToolTipText = function.ToString();
            return item;
        }

        [Flags]
        enum RefreshFlags
        {
            None = 0,
            SkipNames = 1 << 0,
            SkipType = 1 << 1,
        }

        private void RefreshApi(CApi api, RefreshFlags flags = RefreshFlags.None)
        {
            if (!flags.HasFlag(RefreshFlags.SkipNames))
            {
                tbSystemName.Text = api.SystemName;
            }

            if (!flags.HasFlag(RefreshFlags.SkipType))
            {
                cbFixedType.Checked = api.Type == CApiType.Fixed;
                cbDynamicType.Checked = api.Type == CApiType.Dynamic;
            }

            lvFunctions.BeginUpdate();
            lvFunctions.Items.Clear();
            foreach (CFunction function in api.Functions)
            {
                ListViewItem item = AddFunctionItem(function);
            }
            lvFunctions.EndUpdate();

            lbImplementations.BeginUpdate();
            lbImplementations.Items.Clear();
            foreach (CImplementation impl in api.Implementations)
            {
                lbImplementations.Items.Add(impl.Name);
            }
            lbImplementations.EndUpdate();
        }

        private void cbFixedType_Click(object sender, EventArgs e)
        {
            cbFixedType.Checked = true;
            cbDynamicType.Checked = false;
            _api = ConvertApiType(_api, CApiType.Fixed);
            RefreshApi(_api, RefreshFlags.SkipType);
            Generate();
        }

        private void cbDynamicType_Click(object sender, EventArgs e)
        {
            cbDynamicType.Checked = true;
            cbFixedType.Checked = false;
            _api = ConvertApiType(_api, CApiType.Dynamic);
            RefreshApi(_api, RefreshFlags.SkipType);
            Generate();
        }

        private void tbSystemName_TextChanged(object sender, EventArgs e)
        {
            string systemName = tbSystemName.Text;
            if (string.IsNullOrWhiteSpace(systemName) || !CApi.NameRex.IsMatch(systemName))
                return;
            _api.Rename(tbSystemName.Text);
            RefreshApi(_api, RefreshFlags.SkipNames);
            Generate();
        }
    }
}
