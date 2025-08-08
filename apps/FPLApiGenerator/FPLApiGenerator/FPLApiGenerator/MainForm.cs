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
            rtbDefines.Clear();
            rtbPublicAPI.Clear();
            rtbPrivateAPI.Clear();
            rtbImplementation.Clear();
        }

        private static string CreateSeparator(char c, int count)
        {
            string s = new string(c, count);
            return s;
        }

        class ApiEditor
        {
            private readonly RichTextBox _rtb;

            public ApiEditor(RichTextBox rtb)
            {
                _rtb = rtb ?? throw new ArgumentNullException(nameof(rtb));
            }

            private void Append(ReadOnlySpan<char> input) => _rtb.AppendText(input.ToString());

            public void Add(ReadOnlySpan<char> input) => Append(input);
            public void AddLine(ReadOnlySpan<char> input)
            {
                if (input.Length > 0)
                    Append(input);
                Append(Environment.NewLine);
            }
            public void AddLine() => Append(Environment.NewLine);
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

            string funcPostfixUpper = CApi.FuncPostfix.ToUpper();
            string funcPrefixUpper = CApi.FuncPrefix.ToUpper();

            string initFlagsEnumName = $"fplInitFlags";

            string initFlagsValueName = $"{initFlagsEnumName}_{systemName}";

            string backendTypeEnumName = $"{CApi.PublicTypePrefix}{backendName}Type";

            string settingsName = $"{systemName}Settings";

            string settingsStructName = $"{CApi.PublicTypePrefix}{settingsName}";

            IndentState indentState = new IndentState();

            ApiEditor defines = new ApiEditor(rtbDefines);
            ApiEditor publicApi = new ApiEditor(rtbPublicAPI);
            ApiEditor privateAPI = new ApiEditor(rtbPrivateAPI);
            ApiEditor internalBackendAPI = new ApiEditor(rtbImplementation);
            ApiEditor implementation = new ApiEditor(rtbImplementation);
            ApiEditor platformStates = new ApiEditor(rtbPrivateAPI);

            //
            // Defines
            //
            string backendsDefineName = $"{CApi.PublicDefinePrefix}{upperSystemName}";
            string noDefineName = $"{CApi.PublicDefinePrefix}NO_{upperSystemName}";
            string supportDefineName = $"{CApi.InternalDefinePrefix}SUPPORT_{upperSystemName}";
            string enableDefineName = $"{CApi.InternalDefinePrefix}ENABLE_{upperSystemName}";
            defines.AddLine($"#if !defined({noDefineName})");
            defines.AddLine($"#define {supportDefineName}");
            defines.AddLine($"#endif // !{noDefineName}");
            defines.AddLine();
            defines.AddLine($"#if defined({supportDefineName})");
            defines.AddLine($"#define {enableDefineName}");
            defines.AddLine($"#endif // {supportDefineName}");

            //
            // Public API
            //
            publicApi.AddLine($"// {definitionSeperator}");
            publicApi.AddLine($"//");
            publicApi.AddLine($"// > Public API");
            publicApi.AddLine($"//");
            publicApi.AddLine($"// {definitionSeperator}");
            publicApi.AddLine();

            publicApi.AddLine("// Only add the new init flags to the enum");
            publicApi.AddLine($"typedef enum {initFlagsEnumName} {{");
            using (var indent = new Indent(indentState))
            {
                publicApi.AddLine($"{indent}{initFlagsEnumName}_None = 0,");
                publicApi.AddLine();
                publicApi.AddLine($"{indent}{initFlagsValueName} = 1 << 31,");
                publicApi.AddLine();
                publicApi.AddLine($"{indent}{initFlagsEnumName}_All = {initFlagsValueName},");
            }
            publicApi.AddLine($"}} {initFlagsEnumName};");
            publicApi.AddLine();

            publicApi.AddLine($"// {systemName} backend type");
            publicApi.AddLine($"typedef enum {backendTypeEnumName} {{");
            using (var indent = new Indent(indentState))
            {
                publicApi.AddLine($"{indent}{backendTypeEnumName}_None = 0,");
                foreach (CImplementation impl in api.Implementations)
                    publicApi.AddLine($"{indent}{backendTypeEnumName}_{impl.Name},");
            }
            publicApi.AddLine($"}} {backendTypeEnumName};");
            publicApi.AddLine();

            publicApi.AddLine($"// {systemName} backend settings");
            publicApi.AddLine($"typedef struct {settingsStructName} {{");
            using (var indent = new Indent(indentState))
            {
                publicApi.AddLine($"{indent}{backendTypeEnumName} type;");
            }
            publicApi.AddLine($"}} {settingsStructName};");
            publicApi.AddLine();


            //
            // Internal backend API
            //
            string apiImplementedDefineName = $"{backendDefineName}_API_IMPLEMENTED";
            internalBackendAPI.AddLine($"// {definitionSeperator}");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// > {backendDefineName} API");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// {definitionSeperator}");
            internalBackendAPI.AddLine($"#if !defined({apiImplementedDefineName}) && defined({enableDefineName})");
            internalBackendAPI.AddLine($"#define {apiImplementedDefineName}");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// {systemName} backend forward declarations");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine();
            internalBackendAPI.AddLine($"{api.ContextType};");
            internalBackendAPI.AddLine($"{api.BackendType};");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// {systemName} backend function definitions");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine();

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
                internalBackendAPI.Add($"#define {functionDefineName}(name) {resultType} name(");

                if (function.Arguments.Length == 0)
                    internalBackendAPI.Add("void");
                else
                {
                    StringBuilder args = new StringBuilder();
                    foreach (CArgument argument in function.Arguments)
                    {
                        if (args.Length > 0)
                            args.Append(", ");
                        args.Append(argument);
                    }
                    internalBackendAPI.Add(args.ToString());
                }

                internalBackendAPI.AddLine(")");

                internalBackendAPI.AddLine($"typedef {functionDefineName}({functionTypedefName});");
                internalBackendAPI.AddLine();

                functionTableFieldNamesMap.Add(function, functionTableFieldName);
                functionTableDefineNamesMap.Add(function, functionDefineName);
            }

            string tableStructName = $"{CApi.PublicTypePrefix}{backendName}Table";
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// {systemName} backend function table");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"typedef struct {tableStructName} {{");
            using (var indent = new Indent(indentState))
            {
                foreach (CFunction function in api.Functions)
                {
                    string functionName = function.Name;
                    string funcTypeName = $"{CApi.InternalFunctionPrefix}{CApi.FuncPrefix}{systemName}_backend_{functionName}{funcPostfixUpper}";
                    string fieldName = functionTableFieldNamesMap[function];
                    internalBackendAPI.AddLine($"{indent}{funcTypeName} *{fieldName};");
                }
            }
            internalBackendAPI.AddLine($"}} {tableStructName};");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine($"// {systemName} backend descriptor");
            internalBackendAPI.AddLine($"//");
            internalBackendAPI.AddLine();

            string idTypeName = $"{CApi.PublicTypePrefix}{backendName}Id";
            internalBackendAPI.AddLine($"// 8-CC code of the {systemName} backend");
            internalBackendAPI.AddLine($"typedef uint64_t {idTypeName};");
            internalBackendAPI.AddLine();

            string idNameStructName = $"{CApi.PublicTypePrefix}{backendName}IdName";
            CArgument idnameNameArgument = new CArgument(CType.StringType, "name");
            internalBackendAPI.AddLine($"typedef struct {idNameStructName} {{");
            using (var indent = new Indent(indentState))
            {
                internalBackendAPI.AddLine($"{indent}{idTypeName} id;");
                internalBackendAPI.AddLine($"{indent}{idnameNameArgument};");
                internalBackendAPI.AddLine($"}} {idNameStructName};");
            }
            internalBackendAPI.AddLine();

            string headerStructName = $"{CApi.PublicTypePrefix}{backendName}Header";
            CArgument sizeArgument = new CArgument(CType.SizeType, "size");
            CArgument isValidArgument = new CArgument(CType.B32Type, "isValid");
            internalBackendAPI.AddLine($"typedef struct {headerStructName} {{");
            using (var indent = new Indent(indentState))
            {
                internalBackendAPI.AddLine($"{indent}{idNameStructName} idName;");
                internalBackendAPI.AddLine($"{indent}{backendTypeEnumName} type;");
                internalBackendAPI.AddLine($"{indent}{sizeArgument};");
                internalBackendAPI.AddLine($"{indent}{isValidArgument};");
            }
            internalBackendAPI.AddLine($"}} {headerStructName};");
            internalBackendAPI.AddLine();

            string descriptorStructName = $"{CApi.PublicTypePrefix}{backendName}Descriptor";
            internalBackendAPI.AddLine($"typedef struct {descriptorStructName} {{");
            using (var indent = new Indent(indentState))
            {
                internalBackendAPI.AddLine($"{indent}{headerStructName} header;");
                internalBackendAPI.AddLine($"{indent}{tableStructName} table;");
            }
            internalBackendAPI.AddLine($"}} {descriptorStructName};");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"// Stores internal data for the {systemName} backend");
            internalBackendAPI.AddLine($"typedef struct {api.BackendType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                internalBackendAPI.AddLine($"{indent}// Unused to prevent compile errors;");
                internalBackendAPI.AddLine($"{indent}int unused;");
            }
            internalBackendAPI.AddLine($"}} {api.BackendType.Name};");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"#define {backendDataPaddingDefineName} 16");
            internalBackendAPI.AddLine($"#define {backendDataOffsetDefineName} (sizeof({api.BackendType.Name}) + {backendDataPaddingDefineName})");
            internalBackendAPI.AddLine($"#define {backendImplDefineName}(backend, type) (type *)(((uint8_t *)(backend) + {backendDataOffsetDefineName}))");
            internalBackendAPI.AddLine();

            internalBackendAPI.AddLine($"#endif // {apiImplementedDefineName} && {enableDefineName}");

            internalBackendAPI.AddLine();
            internalBackendAPI.AddLine();
            internalBackendAPI.AddLine();

            //
            // Backends implementations
            //
            string commonImplementedDefineName = $"{backendDefineName}S_IMPLEMENTED";
            implementation.AddLine($"// {definitionSeperator}");
            implementation.AddLine($"//");
            implementation.AddLine($"// > {backendDefineName}S");
            implementation.AddLine($"//");
            implementation.AddLine($"// {definitionSeperator}");
            implementation.AddLine($"#if !defined({commonImplementedDefineName}) && defined({enableDefineName})");
            implementation.AddLine($"#define {commonImplementedDefineName}");
            implementation.AddLine();

            implementation.AddLine($"// Stores common data for the {systemName} backend");
            implementation.AddLine($"typedef struct {api.ContextType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                implementation.AddLine($"{indent}// Unused to prevent compile errors;");
                implementation.AddLine($"{indent}int unused;");
            }
            implementation.AddLine($"}} {api.ContextType.Name};");
            implementation.AddLine();

            implementation.AddLine($"// Stores the table and all relevant pointers and data for the {systemName} backend");
            implementation.AddLine($"typedef struct {api.CommonType.Name} {{");
            using (var indent = new Indent(indentState))
            {
                implementation.AddLine($"{indent}// Function table;");
                implementation.AddLine($"{indent}{tableStructName} table;");
                implementation.AddLine($"{indent}// Context;");
                implementation.AddLine($"{indent}{api.ContextType.Name} context;");
                implementation.AddLine($"{indent}// Reference to the backend;");
                implementation.AddLine($"{indent}{api.BackendType.Name} *backend;");
            }
            implementation.AddLine($"}} {api.CommonType.Name};");
            implementation.AddLine();

            foreach (CImplementation impl in api.Implementations)
            {
                EightCC code = impl.Code;

                string implUpperName = impl.Name.ToUpper();

                string enableImplDefineName = $"{enableDefineName}";

                string descriptorImplName = $"{backendName}{impl.Name}Descriptor";

                implementation.AddLine($"// {implementationSeperator}");
                implementation.AddLine("//");
                implementation.AddLine($"// > {backendDefineName}_{implUpperName}");
                implementation.AddLine("//");
                implementation.AddLine($"// {implementationSeperator}");
                implementation.AddLine();

                Dictionary<CFunction, string> functionImplementationNameMap = new Dictionary<CFunction, string>();

                foreach (CFunction function in api.Functions)
                {
                    string functionName = function.Name;
                    string upperFunctionName = functionName.ToUpper();
                    string functionDefineName = functionTableDefineNamesMap[function];

                    string functionImplementationName = $"{CApi.InternalFunctionPrefix}{systemName}_Backend_{impl.Name}_{functionName}";

                    implementation.AddLine($"//");
                    implementation.AddLine($"// {functionName} of the {impl.Name} backend");
                    implementation.AddLine($"//");
                    implementation.AddLine($"{CApi.InternalApiCall} {functionDefineName}({functionImplementationName}) {{");
                    using (var indent = new Indent(indentState))
                    {
                        if (function.ResultType == CType.VoidType)
                            implementation.AddLine();
                        else
                        {
                            string defaultValue = CValue.GetDefaultValue(function.ResultType);
                            implementation.AddLine($"{indent} return {defaultValue};");
                        }
                    }
                    implementation.AddLine($"}}");
                    implementation.AddLine();

                    functionImplementationNameMap.Add(function, functionImplementationName);
                }

                implementation.AddLine($"// Descriptor table of the {impl.Name} backend");
                implementation.AddLine($"{CApi.GlobalVariable} {descriptorStructName} {CApi.InternalFunctionPrefix}global_{descriptorImplName} = {{");
                using (var indent1 = new Indent(indentState))
                {
                    implementation.AddLine($"{indent1}fplStructField({descriptorStructName}, header, {{");
                    using (var indent2 = new Indent(indentState))
                    {
                        implementation.AddLine($"{indent2}fplStructField({headerStructName}, idName, {{");
                        using (var indent3 = new Indent(indentState))
                        {
                            implementation.AddLine($"{indent3}fplStructField({idNameStructName}, id, 0x{code.ToHex()}),");
                            implementation.AddLine($"{indent3}fplStructField({idNameStructName}, name, \"{impl.Name}\"),");
                        }
                        implementation.AddLine($"{indent2}}}),");
                        implementation.AddLine($"{indent2}fplStructField({headerStructName}, type, {backendTypeEnumName}_{impl.Name}),");
                        implementation.AddLine($"{indent2}fplStructField({headerStructName}, isValid, true),");
                    }
                    implementation.AddLine($"{indent1}}}),");
                    implementation.AddLine($"{indent1}fplStructField({descriptorStructName}, table, {{");
                    foreach (CFunction function in api.Functions)
                    {
                        string functionFieldName = functionTableFieldNamesMap[function];
                        string functionImplementationName = functionImplementationNameMap[function];
                        using (var indent2 = new Indent(indentState))
                        {
                            implementation.AddLine($"{indent2}fplStructField({tableStructName}, {functionFieldName}, {functionImplementationName}),");
                        }
                    }
                    implementation.AddLine($"{indent1}}}),");
                }
                implementation.AddLine($"}};");
                implementation.AddLine();
            }

            implementation.AddLine($"#endif // {commonImplementedDefineName} && {enableDefineName}");
            implementation.AddLine();

            //
            // Platform States
            //
            string platformStatesDefined = $"{CApi.InternalDefinePrefix}PLATFORM_STATES_DEFINED";
            platformStates.AddLine($"// {implementationSeperator}");
            platformStates.AddLine($"//");
            platformStates.AddLine($"// > PLATFORM_STATES");
            platformStates.AddLine($"//");
            platformStates.AddLine($"// {implementationSeperator}");
            platformStates.AddLine($"#if !defined({platformStatesDefined})");
            platformStates.AddLine($"#define {platformStatesDefined}");
            platformStates.AddLine();

            string platformMemoryBlockStruct = $"{CApi.InternalFunctionPrefix}PlatformMemoryBlock";
            platformStates.AddLine("// Platform Memory Block (Do not change)");
            platformStates.AddLine($"typedef struct {{");
            using (var indent = new Indent(indentState))
            {
                platformStates.AddLine($"{indent}{CType.SizeType} size;");
                platformStates.AddLine($"{indent}{CType.UPtrType} offset;");
            }
            platformStates.AddLine($"}} {platformMemoryBlockStruct};");
            platformStates.AddLine();

            string backendStateStruct = $"{CApi.InternalFunctionPrefix}PlatformBackendState";
            platformStates.AddLine("// Platform Backend State (Do not change)");
            platformStates.AddLine($"typedef struct {{");
            using (var indent = new Indent(indentState))
            {
                platformStates.AddLine($"{indent}{CType.VoidType} *mem;");
                platformStates.AddLine($"{indent}{CType.SizeType} *size;");
                platformStates.AddLine($"{indent}{CType.SizeType} maxBackendSize;");
                platformStates.AddLine($"{indent}{CType.SizeType} offsetToBackend;");
            }
            platformStates.AddLine($"}} {backendStateStruct};");
            platformStates.AddLine();

            string platformAppStateStruct = $"{CApi.InternalFunctionPrefix}PlatformAppState";
            platformStates.AddLine("// Platform Application State");
            platformStates.AddLine($"typedef struct {platformAppStateStruct} {platformAppStateStruct};");
            platformStates.AddLine($"struct {platformAppStateStruct} {{");
            platformStates.AddLine($"#if defined({enableDefineName})");
            using (var indent = new Indent(indentState))
            {
                platformStates.AddLine($"{indent}{backendStateStruct} {systemNameCamelCase};");
            }
            platformStates.AddLine($"#endif");
            platformStates.AddLine($"}};");
            platformStates.AddLine();

            string backendMemoryBlockVariableName = $"{systemNameCamelCase}MemoryBlock";
            platformStates.AddLine($"fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {{");
            using (var indent1 = new Indent(indentState))
            {
                platformStates.AddLine($"{indent1}//");
                platformStates.AddLine($"{indent1}// Compute platform memory");
                platformStates.AddLine($"{indent1}//");
                platformStates.AddLine();
                platformStates.AddLine($"{indent1}{platformMemoryBlockStruct} {backendMemoryBlockVariableName} = fplZeroInit;");
                platformStates.AddLine();

                string settingsVariableName = $"{systemNameCamelCase}Settings";
                string maxBackendSizeVariableName = $"max{backendName}Size";
                string offsetToBackendVariableName = $"offsetTo{backendName}";
                platformStates.AddLine($"{indent1}// Compute {systemName} backend memory");
                platformStates.AddLine($"#\tif defined({enableDefineName})");
                platformStates.AddLine($"{indent1}{CType.SizeType} {maxBackendSizeVariableName} = 0;");
                platformStates.AddLine($"{indent1}{CType.SizeType} {offsetToBackendVariableName} = 0;");
                platformStates.AddLine($"{indent1}if (fplIsMaskSet(initFlags, {initFlagsValueName})) {{");
                using (var indent2 = new Indent(indentState))
                {
                    platformStates.AddLine($"{indent2}{settingsStructName} {settingsVariableName} = fplZeroInit;");
                    platformStates.AddLine($"{indent2}if (initSettings != fpl_null) {{");
                    using (var indent3 = new Indent(indentState))
                    {
                        platformStates.AddLine($"{indent3}{settingsVariableName} = initSettings->{systemNameCamelCase}");
                    }
                    platformStates.AddLine($"{indent2}}} else {{");
                    using (var indent3 = new Indent(indentState))
                    {
                        platformStates.AddLine($"{indent3}fplSetDefault{settingsName}(&{settingsVariableName});");
                    }
                    platformStates.AddLine($"{indent2}}}");
                    platformStates.AddLine($"{indent2}fpl__PushPlatformMemory(${backendMemoryBlockVariableName}, {maxBackendSizeVariableName}, 16, 0);");
                    platformStates.AddLine($"{indent2}{offsetToBackendVariableName} = 0;");
                }
                platformStates.AddLine($"{indent1}}}");
                platformStates.AddLine($"#{indent1}endif // {enableDefineName}");
            }
            platformStates.AddLine($"}}");

            platformStates.AddLine($"#endif // {platformStatesDefined}");
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
