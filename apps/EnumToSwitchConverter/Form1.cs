using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace EnumToSwitchConverter
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void btnConvert_Click(object sender, EventArgs e)
        {
            string[] sourceItems = tbSource.Text.Split(new string[] { ",", Environment.NewLine }, StringSplitOptions.None);
            List<(string, string)> destItems = new List<(string, string)>();
            foreach (string sourceItem in sourceItems)
            {
                string name = sourceItem.Trim();
                if (name.Length == 0) continue;
                if (name.IndexOf('=') > -1)
                    name = name.Substring(0, name.IndexOf('='));
                name = name.Trim();
                destItems.Add((name, name));
            }

            StringBuilder dest = new StringBuilder();
            dest.AppendLine("switch (value) {");
            foreach (var destItem in destItems)
            {
                dest.AppendLine($"\tcase {destItem.Item1}:");
                dest.AppendLine($"\t\treturn \"{destItem.Item2}\";");
            }
            dest.AppendLine("\tdefault:");
            dest.AppendLine("\t\treturn \"Unknown\";");
            dest.AppendLine("}");
            tbDest.Text = dest.ToString();
        }
    }
}
