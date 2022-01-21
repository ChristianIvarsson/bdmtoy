namespace bdmstuff
{
    partial class frmMain
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
            this.btnFlash = new System.Windows.Forms.Button();
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.cbxEcuType = new System.Windows.Forms.ComboBox();
            this.btnDump = new System.Windows.Forms.Button();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.lblFlash = new System.Windows.Forms.Label();
            this.lblEep = new System.Windows.Forms.Label();
            this.btnEeRead = new System.Windows.Forms.Button();
            this.btnEeWrite = new System.Windows.Forms.Button();
            this.lblSrm = new System.Windows.Forms.Label();
            this.btnSrmRead = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnFlash
            // 
            this.btnFlash.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnFlash.Location = new System.Drawing.Point(843, 78);
            this.btnFlash.Name = "btnFlash";
            this.btnFlash.Size = new System.Drawing.Size(107, 50);
            this.btnFlash.TabIndex = 74;
            this.btnFlash.Text = "Flash";
            this.btnFlash.UseVisualStyleBackColor = true;
            this.btnFlash.Click += new System.EventHandler(this.btnFlash_Click);
            // 
            // listBoxLog
            // 
            this.listBoxLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listBoxLog.Font = new System.Drawing.Font("Courier New", 12.25F);
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.ItemHeight = 18;
            this.listBoxLog.Location = new System.Drawing.Point(12, 12);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.Size = new System.Drawing.Size(669, 364);
            this.listBoxLog.TabIndex = 75;
            // 
            // cbxEcuType
            // 
            this.cbxEcuType.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbxEcuType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbxEcuType.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F);
            this.cbxEcuType.FormattingEnabled = true;
            this.cbxEcuType.Location = new System.Drawing.Point(687, 12);
            this.cbxEcuType.Name = "cbxEcuType";
            this.cbxEcuType.Size = new System.Drawing.Size(263, 28);
            this.cbxEcuType.TabIndex = 76;
            this.cbxEcuType.SelectedIndexChanged += new System.EventHandler(this.cbxEcuType_SelectedIndexChanged);
            // 
            // btnDump
            // 
            this.btnDump.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnDump.Location = new System.Drawing.Point(730, 78);
            this.btnDump.Name = "btnDump";
            this.btnDump.Size = new System.Drawing.Size(107, 50);
            this.btnDump.TabIndex = 77;
            this.btnDump.Text = "Dump";
            this.btnDump.UseVisualStyleBackColor = true;
            this.btnDump.Click += new System.EventHandler(this.btnDump_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBar1.Location = new System.Drawing.Point(687, 353);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(263, 23);
            this.progressBar1.TabIndex = 78;
            // 
            // lblFlash
            // 
            this.lblFlash.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblFlash.AutoSize = true;
            this.lblFlash.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblFlash.Location = new System.Drawing.Point(727, 59);
            this.lblFlash.Name = "lblFlash";
            this.lblFlash.Size = new System.Drawing.Size(46, 16);
            this.lblFlash.TabIndex = 79;
            this.lblFlash.Text = "Flash";
            // 
            // lblEep
            // 
            this.lblEep.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblEep.AutoSize = true;
            this.lblEep.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblEep.Location = new System.Drawing.Point(727, 130);
            this.lblEep.Name = "lblEep";
            this.lblEep.Size = new System.Drawing.Size(72, 16);
            this.lblEep.TabIndex = 80;
            this.lblEep.Text = "EEPROM";
            // 
            // btnEeRead
            // 
            this.btnEeRead.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnEeRead.Location = new System.Drawing.Point(730, 149);
            this.btnEeRead.Name = "btnEeRead";
            this.btnEeRead.Size = new System.Drawing.Size(107, 50);
            this.btnEeRead.TabIndex = 82;
            this.btnEeRead.Text = "Read";
            this.btnEeRead.UseVisualStyleBackColor = true;
            this.btnEeRead.Click += new System.EventHandler(this.btnEeRead_Click);
            // 
            // btnEeWrite
            // 
            this.btnEeWrite.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnEeWrite.Location = new System.Drawing.Point(843, 149);
            this.btnEeWrite.Name = "btnEeWrite";
            this.btnEeWrite.Size = new System.Drawing.Size(107, 50);
            this.btnEeWrite.TabIndex = 81;
            this.btnEeWrite.Text = "Write";
            this.btnEeWrite.UseVisualStyleBackColor = true;
            this.btnEeWrite.Click += new System.EventHandler(this.btnEeWrite_Click);
            // 
            // lblSrm
            // 
            this.lblSrm.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.lblSrm.AutoSize = true;
            this.lblSrm.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSrm.Location = new System.Drawing.Point(727, 202);
            this.lblSrm.Name = "lblSrm";
            this.lblSrm.Size = new System.Drawing.Size(44, 16);
            this.lblSrm.TabIndex = 83;
            this.lblSrm.Text = "Sram";
            // 
            // btnSrmRead
            // 
            this.btnSrmRead.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnSrmRead.Location = new System.Drawing.Point(730, 221);
            this.btnSrmRead.Name = "btnSrmRead";
            this.btnSrmRead.Size = new System.Drawing.Size(107, 50);
            this.btnSrmRead.TabIndex = 84;
            this.btnSrmRead.Text = "Read";
            this.btnSrmRead.UseVisualStyleBackColor = true;
            this.btnSrmRead.Click += new System.EventHandler(this.btnSrmRead_Click);
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(955, 388);
            this.Controls.Add(this.btnSrmRead);
            this.Controls.Add(this.lblSrm);
            this.Controls.Add(this.btnEeRead);
            this.Controls.Add(this.btnEeWrite);
            this.Controls.Add(this.lblEep);
            this.Controls.Add(this.lblFlash);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.btnDump);
            this.Controls.Add(this.cbxEcuType);
            this.Controls.Add(this.listBoxLog);
            this.Controls.Add(this.btnFlash);
            this.MinimumSize = new System.Drawing.Size(288, 300);
            this.Name = "frmMain";
            this.Text = "BDMSTUFF";
            this.Shown += new System.EventHandler(this.frmMain_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnFlash;
        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.ComboBox cbxEcuType;
        private System.Windows.Forms.Button btnDump;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.Label lblFlash;
        private System.Windows.Forms.Label lblEep;
        private System.Windows.Forms.Button btnEeRead;
        private System.Windows.Forms.Button btnEeWrite;
        private System.Windows.Forms.Label lblSrm;
        private System.Windows.Forms.Button btnSrmRead;
    }
}