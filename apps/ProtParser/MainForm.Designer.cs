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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.tbSource = new System.Windows.Forms.TextBox();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.tbTarget = new System.Windows.Forms.TextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.tbLoadLibName = new System.Windows.Forms.TextBox();
            this.tbLoadLibFieldPrefix = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.tbLoadLibHandle = new System.Windows.Forms.TextBox();
            this.tbLoadMacro = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.tbPrefix = new System.Windows.Forms.TextBox();
            this.panel1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tbSource
            // 
            this.tbSource.Dock = System.Windows.Forms.DockStyle.Left;
            this.tbSource.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbSource.Location = new System.Drawing.Point(0, 100);
            this.tbSource.Multiline = true;
            this.tbSource.Name = "tbSource";
            this.tbSource.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbSource.Size = new System.Drawing.Size(344, 299);
            this.tbSource.TabIndex = 0;
            this.tbSource.Text = resources.GetString("tbSource.Text");
            this.tbSource.WordWrap = false;
            this.tbSource.TextChanged += new System.EventHandler(this.tbSource_TextChanged);
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(344, 100);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 299);
            this.splitter1.TabIndex = 1;
            this.splitter1.TabStop = false;
            // 
            // tbTarget
            // 
            this.tbTarget.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbTarget.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbTarget.Location = new System.Drawing.Point(347, 100);
            this.tbTarget.Multiline = true;
            this.tbTarget.Name = "tbTarget";
            this.tbTarget.ReadOnly = true;
            this.tbTarget.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbTarget.Size = new System.Drawing.Size(348, 299);
            this.tbTarget.TabIndex = 2;
            this.tbTarget.WordWrap = false;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(695, 100);
            this.panel1.TabIndex = 3;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.tbLoadLibName);
            this.groupBox1.Controls.Add(this.tbLoadLibFieldPrefix);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.tbLoadLibHandle);
            this.groupBox1.Controls.Add(this.tbLoadMacro);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.tbPrefix);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(695, 100);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Configuration:";
            // 
            // tbLoadLibName
            // 
            this.tbLoadLibName.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbLoadLibName.Location = new System.Drawing.Point(216, 71);
            this.tbLoadLibName.Name = "tbLoadLibName";
            this.tbLoadLibName.Size = new System.Drawing.Size(98, 20);
            this.tbLoadLibName.TabIndex = 7;
            this.tbLoadLibName.Text = "libraryName";
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
            this.tbPrefix.TextChanged += new System.EventHandler(this.tbPrefix_TextChanged);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(695, 399);
            this.Controls.Add(this.tbTarget);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.tbSource);
            this.Controls.Add(this.panel1);
            this.Name = "MainForm";
            this.Text = "Function prototype parser";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.panel1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tbSource;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.TextBox tbTarget;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbPrefix;
        private System.Windows.Forms.TextBox tbLoadMacro;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox tbLoadLibHandle;
        private System.Windows.Forms.TextBox tbLoadLibFieldPrefix;
        private System.Windows.Forms.TextBox tbLoadLibName;
    }
}

