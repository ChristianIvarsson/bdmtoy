using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.ComponentModel;
using NameMeDLL;

namespace bdmstuff
{
    class DLLhandler : infoparser
    {
        private noNameGizmo gizmo = new noNameGizmo();
        public int ECUindex = 0;

        public uint targetEepromSize(int index)
        { return gizmo.returnTargetSizeEEPROM(ECUindex); }
        public uint targetSramSize(int index)
        { return gizmo.returnTargetSizeSRAM(ECUindex); }
        public uint targetFlashSize(int index)
        { return gizmo.returnTargetSizeFLASH(ECUindex); }

        public DLLhandler() {
            gizmo.onTextMessage += forwardText;
            gizmo.onProgress += forwarProgress;
        }
        ~DLLhandler() {
            gizmo.onTextMessage -= forwardText;
            gizmo.onProgress -= forwarProgress;
        }
 
        public List<string> collectECUlist()
        {
            var retList = new List<string>();
            if (gizmo != null)
            {
                for (int i = 1; i <= gizmo.returnNumberOfTargets(); i++)
                {
                    retList.Add(gizmo.returnTargetName(i));
                }
            }
            else
            {
                retList.Add("((Internal fault))");
            }

            return retList;
        }

        private void printGarbageInfo()
        {
            CastInfoEvent(gizmo.returnCoreVersion());
            uint flashSize = gizmo.returnTargetSizeFLASH(ECUindex);
            uint sramSize  = gizmo.returnTargetSizeSRAM(ECUindex);
            uint eepromSz  = gizmo.returnTargetSizeEEPROM(ECUindex);

            if (flashSize > 0)
                CastInfoEvent("Target size (FLASH) : " + flashSize.ToString("D") + " Bytes");
            if (eepromSz > 0)
                CastInfoEvent("Target size (EEPROM): " + eepromSz.ToString("D") + " Bytes");
            if (sramSize > 0)
                CastInfoEvent("Target size (SRAM)  : " + sramSize.ToString("D") + " Bytes");

            CastInfoEvent("Target info         : " + gizmo.returnTargetInfo(ECUindex));
        }

        private void forwardText(object sender, textEventArgs textEvent)
        {
            CastInfoEvent(textEvent.infoText());
        }

        private void forwarProgress(object sender, progressEventArgs progevent)
        {
            CastProgressEvent(progevent.percentage());
        }

        public void Cleanup()
        {
            CastInfoEvent("Closing device");
            Debug.Assert(gizmo != null);
            gizmo.Close();
        }

        public void Dump(object sender, DoWorkEventArgs workEvent)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            string filename = (string)workEvent.Argument;

            CastProgressEvent(0);
            Debug.Assert(gizmo != null);

            Stopwatch sw = new Stopwatch();

            printGarbageInfo();

            sw.Start();
            gizmo.TAP_Dump(ECUindex);
            sw.Stop();
            int ms = sw.Elapsed.Milliseconds + (sw.Elapsed.Seconds * 1000) + (sw.Elapsed.Minutes * 60000);
            CastInfoEvent("Took: " + ms.ToString("D") + " ms");

            // Collect filebytes (if any)
            byte[] buffer = gizmo.returnBufferBytes(ECUindex, false);



            if (buffer != null)
            {
                uint len = gizmo.returnTargetSizeFLASH(ECUindex);
                uint checksum = 0;

                for (uint i = 0; i < len; i++)
                {
                    checksum += buffer[i];
                }

                CastInfoEvent("Checksum: " + checksum.ToString("X08"));

                File.WriteAllBytes(filename, buffer);
            }
            else
            {
                CastInfoEvent("Debug, filebuffer: Received nullpntr");
            }


            /*
            catch (Exception e)
            {
                CastInfoEvent("Could not write file... " + e.Message, ActivityType.ConvertingFile);
                workEvent.Result = false;
            }*/
        }


        public void ReadEeprom(object sender, DoWorkEventArgs workEvent)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            string filename = (string)workEvent.Argument;

            CastProgressEvent(0);
            Debug.Assert(gizmo != null);

            Stopwatch sw = new Stopwatch();

            printGarbageInfo();

            sw.Start();
            gizmo.TAP_ReadEeprom(ECUindex);
            sw.Stop();
            int ms = sw.Elapsed.Milliseconds + (sw.Elapsed.Seconds * 1000) + (sw.Elapsed.Minutes * 60000);
            CastInfoEvent("Took: " + ms.ToString("D") + " ms");

            // Collect filebytes (if any)
            byte[] buffer = gizmo.returnBufferBytes(ECUindex, true);

            if (buffer != null)
            {
                uint len = gizmo.returnTargetSizeEEPROM(ECUindex);
                uint checksum = 0;

                for (uint i = 0; i < len; i++)
                {
                    checksum += buffer[i];
                }

                CastInfoEvent("Checksum: " + checksum.ToString("X08"));

                File.WriteAllBytes(filename, buffer);
            }
            else
            {
                CastInfoEvent("Debug, filebuffer: Received nullpntr");
            }


            /*
            catch (Exception e)
            {
                CastInfoEvent("Could not write file... " + e.Message, ActivityType.ConvertingFile);
                workEvent.Result = false;
            }*/
        }

        public void Flash(object sender, DoWorkEventArgs workEvent)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            string filename = (string)workEvent.Argument;
            Stopwatch sw = new Stopwatch();

            byte[] buffer = File.ReadAllBytes(filename);

            CastProgressEvent(0);
            Debug.Assert(gizmo != null);

            printGarbageInfo();

            sw.Start();
            gizmo.TAP_Flash(ECUindex, buffer);
            sw.Stop();

            int min = sw.Elapsed.Minutes;
            int sec = sw.Elapsed.Seconds;
            int ms  = sw.Elapsed.Milliseconds;
            CastInfoEvent("Took: " + min.ToString("D") + " minutes, " + sec.ToString("D") + " seconds and " + ms.ToString("D") + " ms");
        }

        public void WriteEeprom(object sender, DoWorkEventArgs workEvent)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            string filename = (string)workEvent.Argument;
            Stopwatch sw = new Stopwatch();

            byte[] buffer = File.ReadAllBytes(filename);

            CastProgressEvent(0);
            Debug.Assert(gizmo != null);

            printGarbageInfo();

            sw.Start();
            gizmo.TAP_WriteEeprom(ECUindex, buffer);
            sw.Stop();
            int ms = sw.Elapsed.Milliseconds + (sw.Elapsed.Seconds * 1000);
            CastInfoEvent("Took: " + ms.ToString("D") + " ms");
        }

        public void ReadSram(object sender, DoWorkEventArgs workEvent)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            string filename = (string)workEvent.Argument;

            CastProgressEvent(0);
            Debug.Assert(gizmo != null);

            Stopwatch sw = new Stopwatch();

            printGarbageInfo();

            sw.Start();
            gizmo.TAP_ReadSram(ECUindex);
            sw.Stop();
            int ms = sw.Elapsed.Milliseconds + (sw.Elapsed.Seconds * 1000) + (sw.Elapsed.Minutes * 60000);
            CastInfoEvent("Took: " + ms.ToString("D") + " ms");

            // Collect filebytes (if any)
            byte[] buffer = gizmo.returnBufferBytesSRAM(ECUindex);

            if (buffer != null)
            {
                uint len = gizmo.returnTargetSizeSRAM(ECUindex);
                uint checksum = 0;

                for (uint i = 0; i < len; i++)
                {
                    checksum += buffer[i];
                }

                CastInfoEvent("Checksum: " + checksum.ToString("X08"));

                File.WriteAllBytes(filename, buffer);
            }
            else
            {
                CastInfoEvent("Debug, filebuffer: Received nullpntr");
            }

            /*
            catch (Exception e)
            {
                CastInfoEvent("Could not write file... " + e.Message);
                workEvent.Result = false;
            }*/
        }
    }
}
