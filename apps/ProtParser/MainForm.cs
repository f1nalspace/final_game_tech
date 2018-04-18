using System;
using System.IO;
using System.Windows.Forms;

namespace ProtParser
{
    public partial class MainForm : Form
    {
        private string _activeFilename = null;
        private Preset _activePreset = null;

        public MainForm()
        {
            InitializeComponent();
        }

        private void UpdateTarget()
        {
            Preset preset = new Preset();
            UIToPreset(preset);
            string parsed = PrototypeGenerator.ParseFunctionPrototypes(tbSource.Text, preset);
            tbTarget.Text = parsed;
        }

        private void tbSource_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibHandle_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibName_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadLibFieldPrefix_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbPrefix_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void tbLoadMacro_TextChanged(object sender, EventArgs e)
        {
            UpdateTarget();
        }

        private void fileExitItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void UpdateWindowTitle()
        {
            Text = "Prototype Generator";
            if (!string.IsNullOrEmpty(_activeFilename))
                Text = (Path.GetFileName(_activeFilename) + " - " + Text);
        }

        private void UIToPreset(Preset preset)
        {
            preset.SetProperty("Prefix", tbPrefix.Text);
            preset.SetProperty("LoadMacro", tbLoadMacro.Text);
            preset.SetProperty("LoadLibHandle", tbLoadLibHandle.Text);
            preset.SetProperty("LoadLibName", tbLoadLibName.Text);
            preset.SetProperty("LoadLibFieldPrefix", tbLoadLibFieldPrefix.Text);
            foreach (var line in tbSource.Lines)
                preset.AddSource(line);
        }

        private void NewPreset()
        {
            tbPrefix.Text = string.Empty;
            tbLoadMacro.Text = string.Empty;
            tbLoadLibHandle.Text = string.Empty;
            tbLoadLibName.Text = string.Empty;
            tbLoadLibFieldPrefix.Text = string.Empty;
            tbSource.Clear();

            _activeFilename = null;
            _activePreset = null;
            UpdateWindowTitle();
        }

        private void SavePreset(string filename)
        {
            Preset preset = new Preset();
            UIToPreset(preset);
            preset.Save(filename);

            _activeFilename = filename;
            _activePreset = preset;
            UpdateWindowTitle();
        }

        private void LoadPreset(string filename)
        {
            Preset preset = Preset.Load(filename);
            tbPrefix.Text = preset.GetProperty("Prefix");
            tbLoadMacro.Text = preset.GetProperty("LoadMacro");
            tbLoadLibHandle.Text = preset.GetProperty("LoadLibHandle");
            tbLoadLibName.Text = preset.GetProperty("LoadLibName");
            tbLoadLibFieldPrefix.Text = preset.GetProperty("LoadLibFieldPrefix");
            tbSource.Clear();
            string sources = string.Empty;
            foreach (var source in preset.Sources)
                sources += source + Environment.NewLine;
            tbSource.Text = sources;

            _activeFilename = filename;
            _activePreset = preset;
            UpdateWindowTitle();
        }

        private void fileOpenItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                string filename = openFileDialog.FileName;
                LoadPreset(filename);
            }
        }

        private void fileNewItem_Click(object sender, EventArgs e)
        {
            NewPreset();
        }

        private void fileSaveAsItem_Click(object sender, EventArgs e)
        {
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                SavePreset(saveFileDialog.FileName);
            }
        }
    }
}
