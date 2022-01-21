using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;


namespace bdmstuff
{
    public delegate void DelegateUpdateStatus(infoparser.WorkerInfoEventArgs e);
    public delegate void DelegateProgressStatus(int percentage);


    public partial class frmMain : Form
    {
        public DelegateUpdateStatus   m_DelegateUpdateStatus;
        public DelegateProgressStatus m_DelegateProgressStatus;
        readonly DLLhandler dllhandler = new DLLhandler();


        public frmMain()
        {
            Thread.CurrentThread.Priority = ThreadPriority.AboveNormal;
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(onExit);
            InitializeComponent();

            m_DelegateUpdateStatus   = updateStatusInBox;
            m_DelegateProgressStatus = updateProgress;
            dllhandler.onProgress   += trionicCan_onReadProgress;
            dllhandler.onWorkerInfo += ecuTAP_onTAPInfo;

            SetupListboxWrapping();
            dllhandler.ECUindex = 0;
        }


        private void UpdateProgressStatus(int percentage)
        {
            try
            {
                Invoke(m_DelegateProgressStatus, percentage);
            }
            catch (Exception e)
            {
                AddLogItem("Exception: " + e.Message);
            }
        }


        private void updateProgress(int percentage)
        {
            if (progressBar1.Value != percentage)
            {
                progressBar1.Value = percentage;
            }
        }


        void trionicCan_onReadProgress(object sender, infoparser.ProgressEventArgs e)
        {
            UpdateProgressStatus(e.Percentage);
        }


        private void frmMain_Shown(object sender, EventArgs e)
        {
            enableUserElements(false,0);
            populateECUlist();
            enableUserElements(true,0);
        }


        private void populateECUlist()
        {
            cbxEcuType.Items.Clear();
            cbxEcuType.Items.Add("Select target");
            List<string> supportedECUs =  dllhandler.collectECUlist();

            if (supportedECUs != null)
            {

                foreach (string ECU in supportedECUs)
                {
                    try
                    {
                        cbxEcuType.Items.Add(ECU);
                    }

                    catch (Exception ex)
                    {
                        AddLogItem("Exception: " + ex.Message);
                    }
                }
            }
            cbxEcuType.SelectedItem = "Select target";
            /*
            // Selecting ecu EVERY. SINGLE. TIME you test something while coding drivers is annoying..
            cbxEcuType.SelectedItem = "MPC5566, heck do I know";
            enableUserElements(false, 0);
            Application.DoEvents();
            BackgroundWorker bgWorker;
            bgWorker = new BackgroundWorker();
            bgWorker.DoWork += new DoWorkEventHandler(dllhandler.Dump);
            bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
            bgWorker.RunWorkerAsync("");*/
        }


        private void lst_MeasureItem(object sender, MeasureItemEventArgs e)
        {
            e.ItemHeight = (int)e.Graphics.MeasureString(listBoxLog.Items[e.Index].ToString(), listBoxLog.Font, listBoxLog.Width).Height;
        }


        private void lst_DrawItem(object sender, DrawItemEventArgs e)
        {
            e.DrawBackground();
            e.DrawFocusRectangle();
            if (e.Index >= 0)
                e.Graphics.DrawString(listBoxLog.Items[e.Index].ToString(), e.Font, new SolidBrush(e.ForeColor), e.Bounds);
        }


        private void SetupListboxWrapping()
        {
            listBoxLog.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawVariable;
            listBoxLog.MeasureItem += lst_MeasureItem;
            listBoxLog.DrawItem += lst_DrawItem;
        }


        private void AddLogItem(string str)
        {
            listBoxLog.Items.Add(str);
            while (listBoxLog.Items.Count > 100) listBoxLog.Items.RemoveAt(0);
            listBoxLog.SelectedIndex = listBoxLog.Items.Count - 1;
        }


        private void updateStatusInBox(infoparser.WorkerInfoEventArgs e)
        {
            AddLogItem(e.Info);
        }


        void ecuTAP_onTAPInfo(object sender, infoparser.WorkerInfoEventArgs e)
        {
            try
            {
                Invoke(m_DelegateUpdateStatus, e);
            }
            catch (Exception ex)
            {
                AddLogItem(ex.Message);
            }
        }


        private void enableUserElements(bool enable, int index)
        {
            cbxEcuType.Enabled = enable;

            lblFlash.Enabled   = false;
            btnFlash.Enabled   = false;
            btnDump.Enabled    = false;

            btnEeWrite.Enabled = false;
            btnEeRead.Enabled  = false;
            lblEep.Enabled     = false;

            btnSrmRead.Enabled = false;
            lblSrm.Enabled     = false;

            if (cbxEcuType.SelectedIndex > 0)
            {
                uint eepsupport = dllhandler.targetEepromSize(index);
                uint flssupport = dllhandler.targetFlashSize(index);
                uint srmsupport = dllhandler.targetSramSize(index);

                btnEeWrite.Enabled = (eepsupport > 0) ? enable : false;
                btnEeRead.Enabled  = (eepsupport > 0) ? enable : false;
                lblEep.Enabled     = (eepsupport > 0) ? enable : false;

                lblFlash.Enabled   = (flssupport > 0) ? enable : false;
                btnFlash.Enabled   = (flssupport > 0) ? enable : false;
                btnDump.Enabled    = (flssupport > 0) ? enable : false;

                btnSrmRead.Enabled = (srmsupport > 0) ? enable : false;
                lblSrm.Enabled     = (srmsupport > 0) ? enable : false;

            }
        }


        void ImDone(object sender, RunWorkerCompletedEventArgs e)
        {
            enableUserElements(true, cbxEcuType.SelectedIndex);
        }


        private void btnFlash_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog ofd = new OpenFileDialog() { Filter = "Bin files|*.bin", Multiselect = false })
            {
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    enableUserElements(false, 0);
                    Application.DoEvents();
                    BackgroundWorker bgWorker;
                    bgWorker = new BackgroundWorker();

                    bgWorker.DoWork += new DoWorkEventHandler(dllhandler.Flash);

                    bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
                    bgWorker.RunWorkerAsync(ofd.FileName);
                }
            }
        }


        static void onExit(object sender, EventArgs e)
        {

        }


        private void btnDump_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog sfd = new SaveFileDialog() { Filter = "Bin files|*.bin" })
            {
                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    enableUserElements(false, 0);
                    Application.DoEvents();
                    BackgroundWorker bgWorker;
                    bgWorker = new BackgroundWorker();

                    bgWorker.DoWork += new DoWorkEventHandler(dllhandler.Dump);

                    bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
                    bgWorker.RunWorkerAsync(sfd.FileName);
                }
            }
        }


        private void cbxEcuType_SelectedIndexChanged(object sender, EventArgs e)
        {
            dllhandler.ECUindex = cbxEcuType.SelectedIndex;
            enableUserElements(true, cbxEcuType.SelectedIndex);
        }


        private void btnEeRead_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog sfd = new SaveFileDialog() { Filter = "Bin files|*.bin" })
            {
                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    enableUserElements(false, 0);
                    Application.DoEvents();
                    BackgroundWorker bgWorker;
                    bgWorker = new BackgroundWorker();

                    bgWorker.DoWork += new DoWorkEventHandler(dllhandler.ReadEeprom);

                    bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
                    bgWorker.RunWorkerAsync(sfd.FileName);
                }
            }
        }


        private void btnEeWrite_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog ofd = new OpenFileDialog() { Filter = "Bin files|*.bin", Multiselect = false })
            {
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    enableUserElements(false, 0);
                    Application.DoEvents();
                    BackgroundWorker bgWorker;
                    bgWorker = new BackgroundWorker();

                    bgWorker.DoWork += new DoWorkEventHandler(dllhandler.WriteEeprom);

                    bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
                    bgWorker.RunWorkerAsync(ofd.FileName);
                }
            }
        }


        private void btnSrmRead_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog sfd = new SaveFileDialog() { Filter = "Bin files|*.bin" })
            {
                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    enableUserElements(false, 0);
                    Application.DoEvents();
                    BackgroundWorker bgWorker;
                    bgWorker = new BackgroundWorker();

                    bgWorker.DoWork += new DoWorkEventHandler(dllhandler.ReadSram);

                    bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImDone);
                    bgWorker.RunWorkerAsync(sfd.FileName);
                }
            }
        }
    }
}
