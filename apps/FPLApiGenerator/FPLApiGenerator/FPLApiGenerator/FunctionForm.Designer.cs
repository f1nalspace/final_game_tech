namespace FPLApiGenerator
{
    partial class FunctionForm
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
            components = new System.ComponentModel.Container();
            gbHeader = new System.Windows.Forms.GroupBox();
            cbType = new System.Windows.Forms.ComboBox();
            label2 = new System.Windows.Forms.Label();
            btnGenerate = new System.Windows.Forms.Button();
            label1 = new System.Windows.Forms.Label();
            tbName = new System.Windows.Forms.TextBox();
            gbArguments = new System.Windows.Forms.GroupBox();
            splitContainer1 = new System.Windows.Forms.SplitContainer();
            groupBox3 = new System.Windows.Forms.GroupBox();
            btnUpdateArgument = new System.Windows.Forms.Button();
            btnAddArgument = new System.Windows.Forms.Button();
            tbArgumentName = new System.Windows.Forms.TextBox();
            label4 = new System.Windows.Forms.Label();
            cbArgumentType = new System.Windows.Forms.ComboBox();
            label3 = new System.Windows.Forms.Label();
            listView1 = new System.Windows.Forms.ListView();
            columnHeader1 = new System.Windows.Forms.ColumnHeader();
            columnHeader2 = new System.Windows.Forms.ColumnHeader();
            cmsArguments = new System.Windows.Forms.ContextMenuStrip(components);
            gbActions = new System.Windows.Forms.GroupBox();
            btnOK = new System.Windows.Forms.Button();
            btnCancel = new System.Windows.Forms.Button();
            btnClearArgument = new System.Windows.Forms.Button();
            gbHeader.SuspendLayout();
            gbArguments.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            groupBox3.SuspendLayout();
            gbActions.SuspendLayout();
            SuspendLayout();
            // 
            // gbHeader
            // 
            gbHeader.Controls.Add(cbType);
            gbHeader.Controls.Add(label2);
            gbHeader.Controls.Add(btnGenerate);
            gbHeader.Controls.Add(label1);
            gbHeader.Controls.Add(tbName);
            gbHeader.Dock = System.Windows.Forms.DockStyle.Top;
            gbHeader.Font = new System.Drawing.Font("Consolas", 9.75F);
            gbHeader.Location = new System.Drawing.Point(10, 10);
            gbHeader.Name = "gbHeader";
            gbHeader.Size = new System.Drawing.Size(941, 125);
            gbHeader.TabIndex = 1;
            gbHeader.TabStop = false;
            // 
            // cbType
            // 
            cbType.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            cbType.Font = new System.Drawing.Font("Consolas", 9.75F);
            cbType.FormattingEnabled = true;
            cbType.Location = new System.Drawing.Point(6, 87);
            cbType.Name = "cbType";
            cbType.Size = new System.Drawing.Size(929, 23);
            cbType.TabIndex = 8;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new System.Drawing.Font("Consolas", 9.75F);
            label2.Location = new System.Drawing.Point(6, 69);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(91, 15);
            label2.TabIndex = 7;
            label2.Text = "Result Type:";
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
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Font = new System.Drawing.Font("Consolas", 9.75F);
            label1.Location = new System.Drawing.Point(6, 19);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(42, 15);
            label1.TabIndex = 1;
            label1.Text = "Name:";
            // 
            // tbName
            // 
            tbName.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            tbName.Font = new System.Drawing.Font("Consolas", 9.75F);
            tbName.Location = new System.Drawing.Point(6, 38);
            tbName.Name = "tbName";
            tbName.Size = new System.Drawing.Size(929, 23);
            tbName.TabIndex = 0;
            // 
            // gbArguments
            // 
            gbArguments.Controls.Add(splitContainer1);
            gbArguments.Dock = System.Windows.Forms.DockStyle.Fill;
            gbArguments.Font = new System.Drawing.Font("Consolas", 9.75F);
            gbArguments.Location = new System.Drawing.Point(10, 135);
            gbArguments.Margin = new System.Windows.Forms.Padding(0);
            gbArguments.Name = "gbArguments";
            gbArguments.Padding = new System.Windows.Forms.Padding(0);
            gbArguments.Size = new System.Drawing.Size(941, 455);
            gbArguments.TabIndex = 2;
            gbArguments.TabStop = false;
            // 
            // splitContainer1
            // 
            splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            splitContainer1.Font = new System.Drawing.Font("Consolas", 9.75F);
            splitContainer1.IsSplitterFixed = true;
            splitContainer1.Location = new System.Drawing.Point(0, 16);
            splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(groupBox3);
            splitContainer1.Panel1.Margin = new System.Windows.Forms.Padding(3);
            splitContainer1.Panel1.Padding = new System.Windows.Forms.Padding(3);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(listView1);
            splitContainer1.Panel2.Margin = new System.Windows.Forms.Padding(3);
            splitContainer1.Panel2.Padding = new System.Windows.Forms.Padding(3);
            splitContainer1.Size = new System.Drawing.Size(941, 439);
            splitContainer1.SplitterDistance = 313;
            splitContainer1.TabIndex = 0;
            // 
            // groupBox3
            // 
            groupBox3.Controls.Add(btnClearArgument);
            groupBox3.Controls.Add(btnUpdateArgument);
            groupBox3.Controls.Add(btnAddArgument);
            groupBox3.Controls.Add(tbArgumentName);
            groupBox3.Controls.Add(label4);
            groupBox3.Controls.Add(cbArgumentType);
            groupBox3.Controls.Add(label3);
            groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox3.Font = new System.Drawing.Font("Consolas", 9.75F);
            groupBox3.Location = new System.Drawing.Point(3, 3);
            groupBox3.Name = "groupBox3";
            groupBox3.Size = new System.Drawing.Size(307, 433);
            groupBox3.TabIndex = 0;
            groupBox3.TabStop = false;
            groupBox3.Text = "Argument";
            // 
            // btnUpdateArgument
            // 
            btnUpdateArgument.Location = new System.Drawing.Point(145, 120);
            btnUpdateArgument.Name = "btnUpdateArgument";
            btnUpdateArgument.Size = new System.Drawing.Size(75, 23);
            btnUpdateArgument.TabIndex = 6;
            btnUpdateArgument.Text = "RefreshApi";
            btnUpdateArgument.UseVisualStyleBackColor = true;
            // 
            // btnAddArgument
            // 
            btnAddArgument.Location = new System.Drawing.Point(226, 120);
            btnAddArgument.Name = "btnAddArgument";
            btnAddArgument.Size = new System.Drawing.Size(75, 23);
            btnAddArgument.TabIndex = 5;
            btnAddArgument.Text = "AppendPublicAPI";
            btnAddArgument.UseVisualStyleBackColor = true;
            // 
            // tbArgumentName
            // 
            tbArgumentName.Location = new System.Drawing.Point(3, 41);
            tbArgumentName.Name = "tbArgumentName";
            tbArgumentName.Size = new System.Drawing.Size(298, 23);
            tbArgumentName.TabIndex = 3;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Font = new System.Drawing.Font("Consolas", 9.75F);
            label4.Location = new System.Drawing.Point(3, 23);
            label4.Name = "label4";
            label4.Size = new System.Drawing.Size(42, 15);
            label4.TabIndex = 2;
            label4.Text = "Name:";
            // 
            // cbArgumentType
            // 
            cbArgumentType.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            cbArgumentType.Font = new System.Drawing.Font("Consolas", 9.75F);
            cbArgumentType.FormattingEnabled = true;
            cbArgumentType.Location = new System.Drawing.Point(3, 91);
            cbArgumentType.Name = "cbArgumentType";
            cbArgumentType.Size = new System.Drawing.Size(298, 23);
            cbArgumentType.TabIndex = 1;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new System.Drawing.Font("Consolas", 9.75F);
            label3.Location = new System.Drawing.Point(3, 73);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(42, 15);
            label3.TabIndex = 0;
            label3.Text = "Type:";
            // 
            // listView1
            // 
            listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2 });
            listView1.Dock = System.Windows.Forms.DockStyle.Fill;
            listView1.Font = new System.Drawing.Font("Consolas", 9.75F);
            listView1.Location = new System.Drawing.Point(3, 3);
            listView1.Name = "listView1";
            listView1.Size = new System.Drawing.Size(618, 433);
            listView1.TabIndex = 0;
            listView1.UseCompatibleStateImageBehavior = false;
            listView1.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            columnHeader1.Text = "Name";
            columnHeader1.Width = 200;
            // 
            // columnHeader2
            // 
            columnHeader2.Text = "Type";
            columnHeader2.Width = 150;
            // 
            // cmsArguments
            // 
            cmsArguments.Name = "cmsArguments";
            cmsArguments.Size = new System.Drawing.Size(61, 4);
            // 
            // gbActions
            // 
            gbActions.Controls.Add(btnCancel);
            gbActions.Controls.Add(btnOK);
            gbActions.Dock = System.Windows.Forms.DockStyle.Bottom;
            gbActions.Location = new System.Drawing.Point(10, 590);
            gbActions.Name = "gbActions";
            gbActions.Size = new System.Drawing.Size(941, 66);
            gbActions.TabIndex = 4;
            gbActions.TabStop = false;
            // 
            // btnOK
            // 
            btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            btnOK.Location = new System.Drawing.Point(860, 22);
            btnOK.Name = "btnOK";
            btnOK.Size = new System.Drawing.Size(75, 23);
            btnOK.TabIndex = 0;
            btnOK.Text = "OK";
            btnOK.UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            btnCancel.Location = new System.Drawing.Point(6, 22);
            btnCancel.Name = "btnCancel";
            btnCancel.Size = new System.Drawing.Size(75, 23);
            btnCancel.TabIndex = 1;
            btnCancel.Text = "Cancel";
            btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnClearArgument
            // 
            btnClearArgument.Location = new System.Drawing.Point(3, 120);
            btnClearArgument.Name = "btnClearArgument";
            btnClearArgument.Size = new System.Drawing.Size(75, 23);
            btnClearArgument.TabIndex = 7;
            btnClearArgument.Text = "Clear";
            btnClearArgument.UseVisualStyleBackColor = true;
            // 
            // FunctionForm
            // 
            AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            ClientSize = new System.Drawing.Size(961, 666);
            Controls.Add(gbArguments);
            Controls.Add(gbActions);
            Controls.Add(gbHeader);
            Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
            Name = "FunctionForm";
            Padding = new System.Windows.Forms.Padding(10);
            Text = "Function";
            gbHeader.ResumeLayout(false);
            gbHeader.PerformLayout();
            gbArguments.ResumeLayout(false);
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            groupBox3.ResumeLayout(false);
            groupBox3.PerformLayout();
            gbActions.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private System.Windows.Forms.GroupBox gbHeader;
        private System.Windows.Forms.Button btnGenerate;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbName;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox cbType;
        private System.Windows.Forms.GroupBox gbArguments;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox cbArgumentType;
        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.TextBox tbArgumentName;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Button btnAddArgument;
        private System.Windows.Forms.Button btnUpdateArgument;
        private System.Windows.Forms.ContextMenuStrip cmsArguments;
        private System.Windows.Forms.GroupBox gbActions;
        private System.Windows.Forms.Button btnOK;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Button btnClearArgument;
    }
}