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
            cmsFunctions = new System.Windows.Forms.ContextMenuStrip(components);
            tsmiAddFunction = new System.Windows.Forms.ToolStripMenuItem();
            rtbOutput = new System.Windows.Forms.RichTextBox();
            splitContainer2 = new System.Windows.Forms.SplitContainer();
            groupBox3 = new System.Windows.Forms.GroupBox();
            lvFunctions = new System.Windows.Forms.ListView();
            columnHeader1 = new System.Windows.Forms.ColumnHeader();
            columnHeader2 = new System.Windows.Forms.ColumnHeader();
            columnHeader3 = new System.Windows.Forms.ColumnHeader();
            groupBox4 = new System.Windows.Forms.GroupBox();
            lbImplementations = new System.Windows.Forms.ListBox();
            groupBox1.SuspendLayout();
            groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            cmsFunctions.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
            splitContainer2.Panel1.SuspendLayout();
            splitContainer2.Panel2.SuspendLayout();
            splitContainer2.SuspendLayout();
            groupBox3.SuspendLayout();
            groupBox4.SuspendLayout();
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
            splitContainer1.Panel2.Controls.Add(rtbOutput);
            splitContainer1.Size = new System.Drawing.Size(1382, 786);
            splitContainer1.SplitterDistance = 459;
            splitContainer1.TabIndex = 1;
            // 
            // cmsFunctions
            // 
            cmsFunctions.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { tsmiAddFunction });
            cmsFunctions.Name = "cmsFunctions";
            cmsFunctions.Size = new System.Drawing.Size(154, 26);
            // 
            // tsmiAddFunction
            // 
            tsmiAddFunction.Name = "tsmiAddFunction";
            tsmiAddFunction.Size = new System.Drawing.Size(153, 22);
            tsmiAddFunction.Text = "Add function...";
            // 
            // rtbOutput
            // 
            rtbOutput.Dock = System.Windows.Forms.DockStyle.Fill;
            rtbOutput.Font = new System.Drawing.Font("Consolas", 9.75F);
            rtbOutput.Location = new System.Drawing.Point(0, 0);
            rtbOutput.Name = "rtbOutput";
            rtbOutput.ReadOnly = true;
            rtbOutput.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
            rtbOutput.Size = new System.Drawing.Size(919, 786);
            rtbOutput.TabIndex = 1;
            rtbOutput.Text = "";
            rtbOutput.WordWrap = false;
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
            cmsFunctions.ResumeLayout(false);
            splitContainer2.Panel1.ResumeLayout(false);
            splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
            splitContainer2.ResumeLayout(false);
            groupBox3.ResumeLayout(false);
            groupBox4.ResumeLayout(false);
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
        private System.Windows.Forms.RichTextBox rtbOutput;
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
    }
}
