using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace bdmstuff
{
    abstract public class infoparser
    {
        public delegate void WorkerInfo(object sender, WorkerInfoEventArgs e);
        public event infoparser.WorkerInfo onWorkerInfo;

        public delegate void Progress(object sender, ProgressEventArgs e);
        public event infoparser.Progress onProgress;

        protected void CastInfoEvent(string info)
        {
            if (onWorkerInfo != null)
            {
                onWorkerInfo(this, new WorkerInfoEventArgs(info));
            }
        }

        protected void CastProgressEvent(int percentage)
        {
            if (onProgress != null)
            {
                onProgress(this, new ProgressEventArgs(percentage));
            }
        }

        public class ProgressEventArgs : System.EventArgs
        {
            private int _percentage;

            public int Percentage
            {
                get { return _percentage; }
                set { _percentage = value; }
            }

            public ProgressEventArgs(int percentage)
            {
                _percentage = percentage;
            }
        }



        public class WorkerInfoEventArgs : System.EventArgs
        {

            private string _info;

            public string Info
            {
                get { return _info; }
                set { _info = value; }
            }

            public WorkerInfoEventArgs(string info)
            {
                _info = info;
            }
        }
    }
}
