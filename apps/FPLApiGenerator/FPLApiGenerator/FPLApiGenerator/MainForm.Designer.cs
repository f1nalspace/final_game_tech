namespace FPLApiGenerator
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            groupBox1 = new System.Windows.Forms.GroupBox();
            label3 = new System.Windows.Forms.Label();
            tbSystemName = new System.Windows.Forms.TextBox();
            btnGenerate = new System.Windows.Forms.Button();
            cbDynamicType = new System.Windows.Forms.CheckBox();
            cbFixedType = new System.Windows.Forms.CheckBox();
            label2 = new System.Windows.Forms.Label();
            groupBox2 = new System.Windows.Forms.GroupBox();
            splitContainer1 = new System.Windows.Forms.SplitContainer();
            splitContainer2 = new System.Windows.Forms.SplitContainer();
            groupBox3 = new System.Windows.Forms.GroupBox();
            lvFunctions = new System.Windows.Forms.ListView();
            columnHeader1 = new System.Windows.Forms.ColumnHeader();
            columnHeader2 = new System.Windows.Forms.ColumnHeader();
            columnHeader3 = new System.Windows.Forms.ColumnHeader();
            cmsFunctions = new System.Windows.Forms.ContextMenuStrip(components);
            tsmiAddFunction = new System.Windows.Forms.ToolStripMenuItem();
            groupBox4 = new System.Windows.Forms.GroupBox();
            lbImplementations = new System.Windows.Forms.ListBox();
            tabControl1 = new System.Windows.Forms.TabControl();
            tabPage1 = new System.Windows.Forms.TabPage();
            rtbPublicAPI = new System.Windows.Forms.RichTextBox();
            tabPage2 = new System.Windows.Forms.TabPage();
            rtbPrivateAPI = new System.Windows.Forms.RichTextBox();
            tabPage3 = new System.Windows.Forms.TabPage();
            rtbImplementation = new System.Windows.Forms.RichTextBox();
            tabPage4 = new System.Windows.Forms.TabPage();
            rtbDefines = new System.Windows.Forms.RichTextBox();
            groupBox1.SuspendLayout();
            groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
            splitContainer2.Panel1.SuspendLayout();
            splitContainer2.Panel2.SuspendLayout();
            splitContainer2.SuspendLayout();
            groupBox3.SuspendLayout();
            cmsFunctions.SuspendLayout();
            groupBox4.SuspendLayout();
            tabControl1.SuspendLayout();
            tabPage1.SuspendLayout();
            tabPage2.SuspendLayout();
            tabPage3.SuspendLayout();
            tabPage4.SuspendLayout();
            SuspendLayout();
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(label3);
            groupBox1.Controls.Add(tbSystemName);
            groupBox1.Controls.Add(btnGenerate);
            groupBox1.Controls.Add(cbDynamicType);
            groupBox1.Controls.Add(cbFixedType);
            groupBox1.Controls.Add(label2);
            groupBox1.Dock = System.Windows.Forms.DockStyle.Top;
            groupBox1.Font = new System.Drawing.Font("Consolas", 9.75F);
            groupBox1.Location = new System.Drawing.Point(10, 11);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(1388, 73);
            groupBox1.TabIndex = 0;
            groupBox1.TabStop = false;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new System.Drawing.Font("Consolas", 9.75F);
            label3.Location = new System.Drawing.Point(6, 17);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(91, 15);
            label3.TabIndex = 7;
            label3.Text = "System Name:";
            // 
            // tbSystemName
            // 
            tbSystemName.Font = new System.Drawing.Font("Consolas", 9.75F);
            tbSystemName.Location = new System.Drawing.Point(6, 36);
            tbSystemName.Name = "tbSystemName";
            tbSystemName.Size = new System.Drawing.Size(364, 23);
            tbSystemName.TabIndex = 6;
            tbSystemName.Text = "GameControllers";
            tbSystemName.TextChanged += tbSystemName_TextChanged;
            // 
            // btnGenerate
            // 
            btnGenerate.Font = new System.Drawing.Font("Consolas", 9.75F);
            btnGenerate.Location = new System.Drawing.Point(1253, 19);
            btnGenerate.Name = "btnGenerate";
            btnGenerate.Size = new System.Drawing.Size(125, 25);
            btnGenerate.TabIndex = 5;
            btnGenerate.Text = "Generate";
            btnGenerate.UseVisualStyleBackColor = true;
            btnGenerate.Click += Generate;
            // 
            // cbDynamicType
            // 
            cbDynamicType.AutoCheck = false;
            cbDynamicType.AutoSize = true;
            cbDynamicType.Checked = true;
            cbDynamicType.CheckState = System.Windows.Forms.CheckState.Checked;
            cbDynamicType.Font = new System.Drawing.Font("Consolas", 9.75F);
            cbDynamicType.Location = new System.Drawing.Point(447, 38);
            cbDynamicType.Name = "cbDynamicType";
            cbDynamicType.Size = new System.Drawing.Size(75, 19);
            cbDynamicType.TabIndex = 4;
            cbDynamicType.Text = "Dynamic";
            cbDynamicType.UseVisualStyleBackColor = true;
            cbDynamicType.Click += cbDynamicType_Click;
            // 
            // cbFixedType
            // 
            cbFixedType.AutoCheck = false;
            cbFixedType.AutoSize = true;
            cbFixedType.Font = new System.Drawing.Font("Consolas", 9.75F);
            cbFixedType.Location = new System.Drawing.Point(380, 38);
            cbFixedType.Name = "cbFixedType";
            cbFixedType.Size = new System.Drawing.Size(61, 19);
            cbFixedType.TabIndex = 3;
            cbFixedType.Text = "Fixed";
            cbFixedType.UseVisualStyleBackColor = true;
            cbFixedType.Click += cbFixedType_Click;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new System.Drawing.Font("Consolas", 9.75F);
            label2.Location = new System.Drawing.Point(380, 17);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(42, 15);
            label2.TabIndex = 2;
            label2.Text = "Type:";
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(splitContainer1);
            groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox2.Font = new System.Drawing.Font("Consolas", 9.75F);
            groupBox2.Location = new System.Drawing.Point(10, 84);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new System.Drawing.Size(1388, 808);
            groupBox2.TabIndex = 1;
            groupBox2.TabStop = false;
            // 
            // splitContainer1
            // 
            splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            splitContainer1.Font = new System.Drawing.Font("Consolas", 9.75F);
            splitContainer1.Location = new System.Drawing.Point(3, 19);
            splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(splitContainer2);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(tabControl1);
            splitContainer1.Size = new System.Drawing.Size(1382, 786);
            splitContainer1.SplitterDistance = 459;
            splitContainer1.TabIndex = 1;
            // 
            // splitContainer2
            // 
            splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            splitContainer2.Location = new System.Drawing.Point(0, 0);
            splitContainer2.Name = "splitContainer2";
            splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            splitContainer2.Panel1.Controls.Add(groupBox3);
            // 
            // splitContainer2.Panel2
            // 
            splitContainer2.Panel2.Controls.Add(groupBox4);
            splitContainer2.Size = new System.Drawing.Size(459, 786);
            splitContainer2.SplitterDistance = 393;
            splitContainer2.TabIndex = 2;
            // 
            // groupBox3
            // 
            groupBox3.Controls.Add(lvFunctions);
            groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox3.Location = new System.Drawing.Point(0, 0);
            groupBox3.Name = "groupBox3";
            groupBox3.Size = new System.Drawing.Size(459, 393);
            groupBox3.TabIndex = 2;
            groupBox3.TabStop = false;
            groupBox3.Text = "Functions";
            // 
            // lvFunctions
            // 
            lvFunctions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2, columnHeader3 });
            lvFunctions.ContextMenuStrip = cmsFunctions;
            lvFunctions.Dock = System.Windows.Forms.DockStyle.Fill;
            lvFunctions.Font = new System.Drawing.Font("Consolas", 9.75F);
            lvFunctions.FullRowSelect = true;
            lvFunctions.GridLines = true;
            lvFunctions.Location = new System.Drawing.Point(3, 19);
            lvFunctions.Name = "lvFunctions";
            lvFunctions.ShowGroups = false;
            lvFunctions.ShowItemToolTips = true;
            lvFunctions.Size = new System.Drawing.Size(453, 371);
            lvFunctions.TabIndex = 1;
            lvFunctions.UseCompatibleStateImageBehavior = false;
            lvFunctions.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            columnHeader1.Text = "Name";
            columnHeader1.Width = 200;
            // 
            // columnHeader2
            // 
            columnHeader2.Text = "Arguments";
            columnHeader2.Width = 150;
            // 
            // columnHeader3
            // 
            columnHeader3.Text = "ResultType";
            columnHeader3.Width = 100;
            // 
            // cmsFunctions
            // 
            cmsFunctions.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { tsmiAddFunction });
            cmsFunctions.Name = "cmsFunctions";
            cmsFunctions.Size = new System.Drawing.Size(225, 26);
            // 
            // tsmiAddFunction
            // 
            tsmiAddFunction.Name = "tsmiAddFunction";
            tsmiAddFunction.Size = new System.Drawing.Size(224, 22);
            tsmiAddFunction.Text = "AppendPublicAPI function...";
            // 
            // groupBox4
            // 
            groupBox4.Controls.Add(lbImplementations);
            groupBox4.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox4.Location = new System.Drawing.Point(0, 0);
            groupBox4.Name = "groupBox4";
            groupBox4.Size = new System.Drawing.Size(459, 389);
            groupBox4.TabIndex = 0;
            groupBox4.TabStop = false;
            groupBox4.Text = "Implementations";
            // 
            // lbImplementations
            // 
            lbImplementations.Dock = System.Windows.Forms.DockStyle.Fill;
            lbImplementations.FormattingEnabled = true;
            lbImplementations.Location = new System.Drawing.Point(3, 19);
            lbImplementations.Name = "lbImplementations";
            lbImplementations.Size = new System.Drawing.Size(453, 367);
            lbImplementations.TabIndex = 0;
            // 
            // tabControl1
            // 
            tabControl1.Controls.Add(tabPage4);
            tabControl1.Controls.Add(tabPage1);
            tabControl1.Controls.Add(tabPage2);
            tabControl1.Controls.Add(tabPage3);
            tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            tabControl1.Location = new System.Drawing.Point(0, 0);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new System.Drawing.Size(919, 786);
            tabControl1.TabIndex = 2;
            // 
            // tabPage1
            // 
            tabPage1.Controls.Add(rtbPublicAPI);
            tabPage1.Location = new System.Drawing.Point(4, 24);
            tabPage1.Name = "tabPage1";
            tabPage1.Padding = new System.Windows.Forms.Padding(3);
            tabPage1.Size = new System.Drawing.Size(911, 758);
            tabPage1.TabIndex = 0;
            tabPage1.Text = "Public API";
            tabPage1.UseVisualStyleBackColor = true;
            // 
            // rtbPublicAPI
            // 
            rtbPublicAPI.Dock = System.Windows.Forms.DockStyle.Fill;
            rtbPublicAPI.Font = new System.Drawing.Font("Consolas", 9.75F);
            rtbPublicAPI.Location = new System.Drawing.Point(3, 3);
            rtbPublicAPI.Name = "rtbPublicAPI";
            rtbPublicAPI.ReadOnly = true;
            rtbPublicAPI.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            rtbPublicAPI.Size = new System.Drawing.Size(905, 752);
            rtbPublicAPI.TabIndex = 2;
            rtbPublicAPI.Text = "";
            rtbPublicAPI.WordWrap = false;
            // 
            // tabPage2
            // 
            tabPage2.Controls.Add(rtbPrivateAPI);
            tabPage2.Location = new System.Drawing.Point(4, 24);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new System.Windows.Forms.Padding(3);
            tabPage2.Size = new System.Drawing.Size(911, 758);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "Private API";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // rtbPrivateAPI
            // 
            rtbPrivateAPI.Dock = System.Windows.Forms.DockStyle.Fill;
            rtbPrivateAPI.Font = new System.Drawing.Font("Consolas", 9.75F);
            rtbPrivateAPI.Location = new System.Drawing.Point(3, 3);
            rtbPrivateAPI.Name = "rtbPrivateAPI";
            rtbPrivateAPI.ReadOnly = true;
            rtbPrivateAPI.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            rtbPrivateAPI.Size = new System.Drawing.Size(905, 752);
            rtbPrivateAPI.TabIndex = 3;
            rtbPrivateAPI.Text = "";
            rtbPrivateAPI.WordWrap = false;
            // 
            // tabPage3
            // 
            tabPage3.Controls.Add(rtbImplementation);
            tabPage3.Location = new System.Drawing.Point(4, 24);
            tabPage3.Name = "tabPage3";
            tabPage3.Padding = new System.Windows.Forms.Padding(3);
            tabPage3.Size = new System.Drawing.Size(911, 758);
            tabPage3.TabIndex = 2;
            tabPage3.Text = "Implementation";
            tabPage3.UseVisualStyleBackColor = true;
            // 
            // rtbImplementation
            // 
            rtbImplementation.Dock = System.Windows.Forms.DockStyle.Fill;
            rtbImplementation.Font = new System.Drawing.Font("Consolas", 9.75F);
            rtbImplementation.Location = new System.Drawing.Point(3, 3);
            rtbImplementation.Name = "rtbImplementation";
            rtbImplementation.ReadOnly = true;
            rtbImplementation.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            rtbImplementation.Size = new System.Drawing.Size(905, 752);
            rtbImplementation.TabIndex = 3;
            rtbImplementation.Text = "";
            rtbImplementation.WordWrap = false;
            // 
            // tabPage4
            // 
            tabPage4.Controls.Add(rtbDefines);
            tabPage4.Location = new System.Drawing.Point(4, 24);
            tabPage4.Name = "tabPage4";
            tabPage4.Padding = new System.Windows.Forms.Padding(3);
            tabPage4.Size = new System.Drawing.Size(911, 758);
            tabPage4.TabIndex = 3;
            tabPage4.Text = "Defines";
            tabPage4.UseVisualStyleBackColor = true;
            // 
            // rtbDefines
            // 
            rtbDefines.Dock = System.Windows.Forms.DockStyle.Fill;
            rtbDefines.Font = new System.Drawing.Font("Consolas", 9.75F);
            rtbDefines.Location = new System.Drawing.Point(3, 3);
            rtbDefines.Name = "rtbDefines";
            rtbDefines.ReadOnly = true;
            rtbDefines.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            rtbDefines.Size = new System.Drawing.Size(905, 752);
            rtbDefines.TabIndex = 3;
            rtbDefines.Text = "";
            rtbDefines.WordWrap = false;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(1408, 903);
            Controls.Add(groupBox2);
            Controls.Add(groupBox1);
            Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
            Name = "MainForm";
            Padding = new System.Windows.Forms.Padding(10, 11, 10, 11);
            Text = "FPL API Generator";
            groupBox1.ResumeLayout(false);
            groupBox1.PerformLayout();
            groupBox2.ResumeLayout(false);
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            splitContainer2.Panel1.ResumeLayout(false);
            splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
            splitContainer2.ResumeLayout(false);
            groupBox3.ResumeLayout(false);
            cmsFunctions.ResumeLayout(false);
            groupBox4.ResumeLayout(false);
            tabControl1.ResumeLayout(false);
            tabPage1.ResumeLayout(false);
            tabPage2.ResumeLayout(false);
            tabPage3.ResumeLayout(false);
            tabPage4.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox cbDynamicType;
        private System.Windows.Forms.CheckBox cbFixedType;
        private System.Windows.Forms.Button btnGenerate;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ContextMenuStrip cmsFunctions;
        private System.Windows.Forms.ToolStripMenuItem tsmiAddFunction;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox tbSystemName;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ListView lvFunctions;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.ListBox lbImplementations;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.RichTextBox rtbPublicAPI;
        private System.Windows.Forms.RichTextBox rtbPrivateAPI;
        private System.Windows.Forms.RichTextBox rtbImplementation;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.RichTextBox rtbDefines;
    }
}
