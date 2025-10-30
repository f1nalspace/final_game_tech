namespace ProtParser
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.tbSource = new System.Windows.Forms.RichTextBox();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.tbTarget = new System.Windows.Forms.RichTextBox();
            this.panConfiguration = new System.Windows.Forms.Panel();
            this.gbConfiguration = new System.Windows.Forms.GroupBox();
            this.tbProcNamePrefix = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.tbExcludedSymbols = new System.Windows.Forms.TextBox();
            this.tbLoadLibName = new System.Windows.Forms.TextBox();
            this.tbLoadLibFieldPrefix = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.tbLoadLibHandle = new System.Windows.Forms.TextBox();
            this.tbLoadMacro = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.tbPrefix = new System.Windows.Forms.TextBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.fileNewItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileOpenItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fileSave = new System.Windows.Forms.ToolStripMenuItem();
            this.fileSaveAsItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.fileExitItem = new System.Windows.Forms.ToolStripMenuItem();
            this.dlgOpenPreset = new System.Windows.Forms.OpenFileDialog();
            this.dlgSavePreset = new System.Windows.Forms.SaveFileDialog();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.tbDLLFilePath = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.btnSelectDLLFilePath = new System.Windows.Forms.Button();
            this.dlgOpenDLL = new System.Windows.Forms.OpenFileDialog();
            this.panConfiguration.SuspendLayout();
            this.gbConfiguration.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tbSource
            // 
            this.tbSource.Dock = System.Windows.Forms.DockStyle.Left;
            this.tbSource.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbSource.Location = new System.Drawing.Point(0, 214);
            this.tbSource.Name = "tbSource";
            this.tbSource.Size = new System.Drawing.Size(506, 448);
            this.tbSource.TabIndex = 0;
            this.tbSource.Text = resources.GetString("tbSource.Text");
            this.tbSource.WordWrap = false;
            this.tbSource.TextChanged += new System.EventHandler(this.tbSource_TextChanged);
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(506, 214);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 448);
            this.splitter1.TabIndex = 1;
            this.splitter1.TabStop = false;
            // 
            // tbTarget
            // 
            this.tbTarget.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbTarget.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbTarget.Location = new System.Drawing.Point(509, 214);
            this.tbTarget.Name = "tbTarget";
            this.tbTarget.ReadOnly = true;
            this.tbTarget.Size = new System.Drawing.Size(806, 448);
            this.tbTarget.TabIndex = 2;
            this.tbTarget.Text = "";
            this.tbTarget.WordWrap = false;
            // 
            // panConfiguration
            // 
            this.panConfiguration.Controls.Add(this.gbConfiguration);
            this.panConfiguration.Dock = System.Windows.Forms.DockStyle.Top;
            this.panConfiguration.Location = new System.Drawing.Point(0, 24);
            this.panConfiguration.Name = "panConfiguration";
            this.panConfiguration.Size = new System.Drawing.Size(1315, 190);
            this.panConfiguration.TabIndex = 3;
            // 
            // gbConfiguration
            // 
            this.gbConfiguration.Controls.Add(this.btnSelectDLLFilePath);
            this.gbConfiguration.Controls.Add(this.tbDLLFilePath);
            this.gbConfiguration.Controls.Add(this.label6);
            this.gbConfiguration.Controls.Add(this.tbProcNamePrefix);
            this.gbConfiguration.Controls.Add(this.label5);
            this.gbConfiguration.Controls.Add(this.label4);
            this.gbConfiguration.Controls.Add(this.tbExcludedSymbols);
            this.gbConfiguration.Controls.Add(this.tbLoadLibName);
            this.gbConfiguration.Controls.Add(this.tbLoadLibFieldPrefix);
            this.gbConfiguration.Controls.Add(this.label3);
            this.gbConfiguration.Controls.Add(this.tbLoadLibHandle);
            this.gbConfiguration.Controls.Add(this.tbLoadMacro);
            this.gbConfiguration.Controls.Add(this.label2);
            this.gbConfiguration.Controls.Add(this.label1);
            this.gbConfiguration.Controls.Add(this.tbPrefix);
            this.gbConfiguration.Dock = System.Windows.Forms.DockStyle.Fill;
            this.gbConfiguration.Location = new System.Drawing.Point(0, 0);
            this.gbConfiguration.Name = "gbConfiguration";
            this.gbConfiguration.Size = new System.Drawing.Size(1315, 190);
            this.gbConfiguration.TabIndex = 4;
            this.gbConfiguration.TabStop = false;
            this.gbConfiguration.Text = "Configuration:";
            // 
            // tbProcNamePrefix
            // 
            this.tbProcNamePrefix.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbProcNamePrefix.Location = new System.Drawing.Point(106, 122);
            this.tbProcNamePrefix.Name = "tbProcNamePrefix";
            this.tbProcNamePrefix.Size = new System.Drawing.Size(577, 20);
            this.tbProcNamePrefix.TabIndex = 11;
            this.toolTip1.SetToolTip(this.tbProcNamePrefix, "Prefix of the procedure");
            this.tbProcNamePrefix.WordWrap = false;
            this.tbProcNamePrefix.TextChanged += new System.EventHandler(this.tbProcNamePrefix_TextChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(37, 125);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(63, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "Proc Name:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(4, 100);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(96, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "Excluded Symbols:";
            // 
            // tbExcludedSymbols
            // 
            this.tbExcludedSymbols.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbExcludedSymbols.Location = new System.Drawing.Point(106, 97);
            this.tbExcludedSymbols.Name = "tbExcludedSymbols";
            this.tbExcludedSymbols.Size = new System.Drawing.Size(577, 20);
            this.tbExcludedSymbols.TabIndex = 8;
            this.toolTip1.SetToolTip(this.tbExcludedSymbols, "Comma seperated list of excluded symbols");
            this.tbExcludedSymbols.WordWrap = false;
            this.tbExcludedSymbols.TextChanged += new System.EventHandler(this.tbExcludedSymbols_TextChanged);
            // 
            // tbLoadLibName
            // 
            this.tbLoadLibName.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbLoadLibName.Location = new System.Drawing.Point(216, 71);
            this.tbLoadLibName.Name = "tbLoadLibName";
            this.tbLoadLibName.Size = new System.Drawing.Size(98, 20);
            this.tbLoadLibName.TabIndex = 7;
            this.tbLoadLibName.Text = "libraryName";
            this.toolTip1.SetToolTip(this.tbLoadLibName, "Variable name of library name");
            this.tbLoadLibName.WordWrap = false;
            this.tbLoadLibName.TextChanged += new System.EventHandler(this.tbLoadLibName_TextChanged);
            // 
            // tbLoadLibFieldPrefix
            // 
            this.tbLoadLibFieldPrefix.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbLoadLibFieldPrefix.Location = new System.Drawing.Point(320, 71);
            this.tbLoadLibFieldPrefix.Name = "tbLoadLibFieldPrefix";
            this.tbLoadLibFieldPrefix.Size = new System.Drawing.Size(106, 20);
            this.tbLoadLibFieldPrefix.TabIndex = 6;
            this.tbLoadLibFieldPrefix.Text = "wapi->user.";
            this.toolTip1.SetToolTip(this.tbLoadLibFieldPrefix, "Expression to the target container");
            this.tbLoadLibFieldPrefix.WordWrap = false;
            this.tbLoadLibFieldPrefix.TextChanged += new System.EventHandler(this.tbLoadLibFieldPrefix_TextChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(13, 74);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(87, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Load library vars:";
            // 
            // tbLoadLibHandle
            // 
            this.tbLoadLibHandle.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbLoadLibHandle.Location = new System.Drawing.Point(106, 71);
            this.tbLoadLibHandle.Name = "tbLoadLibHandle";
            this.tbLoadLibHandle.Size = new System.Drawing.Size(104, 20);
            this.tbLoadLibHandle.TabIndex = 4;
            this.tbLoadLibHandle.Text = "libraryHandle";
            this.toolTip1.SetToolTip(this.tbLoadLibHandle, "Variable name of library handle");
            this.tbLoadLibHandle.WordWrap = false;
            this.tbLoadLibHandle.TextChanged += new System.EventHandler(this.tbLoadLibHandle_TextChanged);
            // 
            // tbLoadMacro
            // 
            this.tbLoadMacro.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbLoadMacro.Location = new System.Drawing.Point(106, 45);
            this.tbLoadMacro.Name = "tbLoadMacro";
            this.tbLoadMacro.Size = new System.Drawing.Size(577, 20);
            this.tbLoadMacro.TabIndex = 3;
            this.tbLoadMacro.Text = "FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN";
            this.toolTip1.SetToolTip(this.tbLoadMacro, "Name of the get function macro");
            this.tbLoadMacro.WordWrap = false;
            this.tbLoadMacro.TextChanged += new System.EventHandler(this.tbLoadMacro_TextChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(34, 48);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(66, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Load macro:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(64, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(36, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Prefix:";
            // 
            // tbPrefix
            // 
            this.tbPrefix.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbPrefix.Location = new System.Drawing.Point(106, 19);
            this.tbPrefix.Name = "tbPrefix";
            this.tbPrefix.Size = new System.Drawing.Size(577, 20);
            this.tbPrefix.TabIndex = 0;
            this.tbPrefix.Text = "FPL__WIN32_FUNC_";
            this.toolTip1.SetToolTip(this.tbPrefix, "Prefix of the prototype define and typedef");
            this.tbPrefix.WordWrap = false;
            this.tbPrefix.TextChanged += new System.EventHandler(this.tbPrefix_TextChanged);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileMenu});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1315, 24);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileMenu
            // 
            this.fileMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileNewItem,
            this.fileOpenItem,
            this.fileSave,
            this.fileSaveAsItem,
            this.toolStripMenuItem1,
            this.fileExitItem});
            this.fileMenu.Name = "fileMenu";
            this.fileMenu.Size = new System.Drawing.Size(37, 20);
            this.fileMenu.Text = "File";
            // 
            // fileNewItem
            // 
            this.fileNewItem.Name = "fileNewItem";
            this.fileNewItem.Size = new System.Drawing.Size(138, 22);
            this.fileNewItem.Text = "New";
            this.fileNewItem.Click += new System.EventHandler(this.fileNewItem_Click);
            // 
            // fileOpenItem
            // 
            this.fileOpenItem.Name = "fileOpenItem";
            this.fileOpenItem.Size = new System.Drawing.Size(138, 22);
            this.fileOpenItem.Text = "Open...";
            this.fileOpenItem.Click += new System.EventHandler(this.fileOpenItem_Click);
            // 
            // fileSave
            // 
            this.fileSave.Name = "fileSave";
            this.fileSave.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.fileSave.Size = new System.Drawing.Size(180, 22);
            this.fileSave.Text = "Save";
            this.fileSave.Click += new System.EventHandler(this.fileSaveItem_Click);
            // 
            // fileSaveAsItem
            // 
            this.fileSaveAsItem.Name = "fileSaveAsItem";
            this.fileSaveAsItem.Size = new System.Drawing.Size(138, 22);
            this.fileSaveAsItem.Text = "Save as...";
            this.fileSaveAsItem.Click += new System.EventHandler(this.fileSaveAsItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(135, 6);
            // 
            // fileExitItem
            // 
            this.fileExitItem.Name = "fileExitItem";
            this.fileExitItem.Size = new System.Drawing.Size(138, 22);
            this.fileExitItem.Text = "Exit";
            this.fileExitItem.Click += new System.EventHandler(this.fileExitItem_Click);
            // 
            // dlgOpenPreset
            // 
            this.dlgOpenPreset.DefaultExt = "txt";
            this.dlgOpenPreset.Filter = "Text files|*.txt";
            // 
            // dlgSavePreset
            // 
            this.dlgSavePreset.DefaultExt = "txt";
            this.dlgSavePreset.Filter = "Text files|*.txt";
            // 
            // tbDLLFilePath
            // 
            this.tbDLLFilePath.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbDLLFilePath.Location = new System.Drawing.Point(106, 148);
            this.tbDLLFilePath.Name = "tbDLLFilePath";
            this.tbDLLFilePath.Size = new System.Drawing.Size(577, 20);
            this.tbDLLFilePath.TabIndex = 13;
            this.toolTip1.SetToolTip(this.tbDLLFilePath, "File Path of the DLL");
            this.tbDLLFilePath.WordWrap = false;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(51, 151);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(49, 13);
            this.label6.TabIndex = 12;
            this.label6.Text = "DLL File:";
            // 
            // btnSelectDLLFilePath
            // 
            this.btnSelectDLLFilePath.Location = new System.Drawing.Point(689, 146);
            this.btnSelectDLLFilePath.Name = "btnSelectDLLFilePath";
            this.btnSelectDLLFilePath.Size = new System.Drawing.Size(29, 23);
            this.btnSelectDLLFilePath.TabIndex = 14;
            this.btnSelectDLLFilePath.Text = "...";
            this.btnSelectDLLFilePath.UseVisualStyleBackColor = true;
            this.btnSelectDLLFilePath.Click += new System.EventHandler(this.btnSelectDLLFilePath_Click);
            // 
            // dlgOpenDLL
            // 
            this.dlgOpenDLL.DefaultExt = "txt";
            this.dlgOpenDLL.Filter = "DLL files|*.dll";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1315, 662);
            this.Controls.Add(this.tbTarget);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.tbSource);
            this.Controls.Add(this.panConfiguration);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "MainForm";
            this.Text = "Prototype Generator";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.panConfiguration.ResumeLayout(false);
            this.gbConfiguration.ResumeLayout(false);
            this.gbConfiguration.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RichTextBox tbSource;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.RichTextBox tbTarget;
        private System.Windows.Forms.Panel panConfiguration;
        private System.Windows.Forms.GroupBox gbConfiguration;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbPrefix;
        private System.Windows.Forms.TextBox tbLoadMacro;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox tbLoadLibHandle;
        private System.Windows.Forms.TextBox tbLoadLibFieldPrefix;
        private System.Windows.Forms.TextBox tbLoadLibName;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileMenu;
        private System.Windows.Forms.ToolStripMenuItem fileNewItem;
        private System.Windows.Forms.ToolStripMenuItem fileOpenItem;
        private System.Windows.Forms.ToolStripMenuItem fileSaveAsItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem fileExitItem;
        private System.Windows.Forms.OpenFileDialog dlgOpenPreset;
        private System.Windows.Forms.SaveFileDialog dlgSavePreset;
        private System.Windows.Forms.ToolStripMenuItem fileSave;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox tbExcludedSymbols;
        private System.Windows.Forms.TextBox tbProcNamePrefix;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.TextBox tbDLLFilePath;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button btnSelectDLLFilePath;
        private System.Windows.Forms.OpenFileDialog dlgOpenDLL;
    }
}

