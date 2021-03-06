#include "Header.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace LibUsbDotNet;
using namespace LibUsbDotNet::Main;
using namespace LibUsbDotNet::WinUsb;
using namespace std;
[DllImport("winmm.dll", EntryPoint = "timeBeginPeriod")]
extern UINT timeBeginPeriod(UINT uMilliseconds);
[DllImport("winmm.dll", EntryPoint = "timeEndPeriod")]
extern UINT timeEndPeriod(UINT uMilliseconds);

[DllImport("winmm.dll", EntryPoint = "timeGetTime")]
extern UINT timeGetTime();

namespace NameMeDLL
{
	UINT lastTime = 0;

	/// TODO:
	noNameGizmo::noNameGizmo()
	{
		isOpen = false;
		this->thisptr = this;
	}
	noNameGizmo::~noNameGizmo()
	{
        timeEndPeriod(1);
        this->CastTextMessage("Destruction!");
		Close();
        if (this->usb_finder)
        {
            this->usb_finder = nullptr;
        }
	}

    String ^noNameGizmo::returnCoreVersion()
    {   return gcnew String(core_VersionString());}

	const UINT noNameGizmo::returnNumberOfTargets()
	{   return core_NoTargets();                  }

	String ^noNameGizmo::returnTargetName(int index)
	{
        if (index > (int)core_NoTargets() || index < 1)
            return "Out of bounds!";
		
        return gcnew String(core_TargetName((UINT)index));
	}
    String ^noNameGizmo::returnTargetInfo(int index)
    {
        if (index > (int)core_NoTargets() || index < 1)
            return "Out of bounds!";

        return gcnew String(core_TargetInfo((UINT)index));
    }
	
    UINT noNameGizmo::returnTargetSizeFLASH(int index)
    {
        return (index > (int)core_NoTargets() || index < 1) ? 0 : core_TargetSizeFLASH((UINT)index);
    }

	UINT noNameGizmo::returnTargetSizeEEPROM(int index)
	{
		return (index > (int)core_NoTargets() || index < 1) ? 0 : core_TargetSizeEEPROM((UINT)index);
	}

    UINT noNameGizmo::returnTargetSizeSRAM(int index)
    {
        return (index > (int)core_NoTargets() || index < 1) ? 0 : core_TargetSizeSRAM((UINT)index);
    }

	void noNameGizmo::Close()
	{
        this->CastTextMessage("Close requested");

        try
        {
            this->write_lock->WaitOne();

            if (this->ep_reader)
            {
                // detach read event
                this->ep_reader->DataReceivedEnabled = false;
                this->ep_reader->DataReceived -=
                    gcnew EventHandler<EndpointDataEventArgs^>
                    (&noNameGizmo::readAdapter);
            }


            // this->ep_writer->Close();

            // release endpoints
            this->ep_reader = nullptr;
            this->ep_writer = nullptr;

            if (isOpen)
            {
                // close devices
                // this->winusb_dev->Close();
                // this->usb_finder->Close();
                
                this->default_dev->Close();
                this->usb_dev->Close();
            }

            // release devices
            this->default_dev = nullptr;
            this->usb_dev = nullptr;

            UsbDevice::Exit();


        }

        // handle exceptions
        catch (...)
        {
            // ignore everything
        }

        // clean up
        finally
        {
            this->write_lock->ReleaseMutex();
        }
	}


    /// And hackjob of the day goes to...
    // Calling managed code from unmanaged C code is ballzy enough so we still have to check what it wants from our end.
    static void handler_CheckStatus()
    {
        noNameGizmo::managedCallback();
    }
    static void handler_CastProgress(int percentage)
    {
        noNameGizmo::managedProgress(percentage);
    }
	static void handler_CastMessage(char *text)
	{
		noNameGizmo::managedString(text);
	}

    static void handler_SendArray(void *ptr, UINT noBytes)
    {
        // Debug::WriteLine("Sending: " + noBytes.ToString("D") + " bytes");
        BYTE *byteptr = (BYTE *)ptr;
        array<BYTE>^ sent = gcnew array <BYTE>(noBytes);

        for (UINT i = 0; i < noBytes; i++)
            sent[i] = *byteptr++;

        noNameGizmo::writeAdapter(sent, 2000);
    }

    void noNameGizmo::managedProgress(int percentage)
    {
        thisptr->CastProgress(percentage);
    }

	void noNameGizmo::managedString(char *text)
	{
		thisptr->CastTextMessage(gcnew String(text));
		// gcnew String(text);
	}

    void noNameGizmo::managedCallback()
    {
        // thisptr->CastTextMessage("Debug: Core called managed code...");

        if (core_ReturnFaultStatus() > 0)
        {
            thisptr->CastTextMessage("Error: " + gcnew String(core_TranslateFault()));

            if (core_ReturnWorkStatus() > 0)
            {
                thisptr->CastTextMessage("Internal bug: Work is done AND there's a fault...");
            }
        }
        else if (core_ReturnWorkStatus() > 0)
        {
            thisptr->CastTextMessage("Debug: " + gcnew String(core_TranslateFault()));
            thisptr->CastTextMessage("We're done!");
        }
        else
        {
            thisptr->CastTextMessage("Damn core used the callback for no good reason (Internal bug)");
            thisptr->CastTextMessage("Smh...");
        }

        thisptr->Close();
    }

    // Private function. Configure core.
    void noNameGizmo::core_SharedSetup(int index)
    {
        // I can't believe that this actually works!
        core_InstallProgress(&handler_CastProgress);
        core_InstallCallback(&handler_CheckStatus);
        core_InstallSendArray(&handler_SendArray);
		core_InstallMessage(&handler_CastMessage);
    }

	
    void noNameGizmo::TAP_Flash(int index, array <BYTE>^ buffer)
    {
        if (index > 0 && index <= (int)core_NoTargets())
        {
			UINT noBytes = returnTargetSizeFLASH(index);
			
			if (buffer->Length == noBytes)
			{
				// Static so that it won't destroy our buffer
				static BYTE testbuffer[1024 * 1024];

				for (uint32_t i = 0; i < noBytes; i++)
				{
					testbuffer[i] = buffer[i];
				}


				Open();

				if (isOpen)
				{
					core_SharedSetup(index);
					core_FLASH((UINT)index, &testbuffer);
				}
			}
			else
			{
				this->CastTextMessage("Filesize does not match target");
			}
        }
        else
        {
            this->CastTextMessage("ECU select index out of bounds");
        }
    }

	void noNameGizmo::TAP_Dump(int index)
	{
		if (index > 0 && index <= (int)core_NoTargets())
		{
			this->Open();

            if (isOpen)
            {
                core_SharedSetup(index);
                core_DumpFLASH((UINT)index);
            }
		}
		else
        {
			this->CastTextMessage("ECU select index out of bounds");
        }
	}

	void noNameGizmo::TAP_ReadEeprom(int index)
	{
		if (index > 0 && index <= (int)core_NoTargets())
		{
			this->Open();

			if (isOpen)
			{
				core_SharedSetup(index);
				core_DumpEEPROM((UINT)index);
			}
		}
		else
		{
			this->CastTextMessage("ECU select index out of bounds");
		}
	}
	void noNameGizmo::TAP_WriteEeprom(int index, array <BYTE>^ buffer)
	{
		if (index > 0 && index <= (int)core_NoTargets())
		{
			UINT noBytes = returnTargetSizeEEPROM(index);

			if (buffer->Length == noBytes)
			{
				// Static so that it won't destroy our buffer
				static BYTE testbuffer[1024 * 1024];

				for (uint32_t i = 0; i < noBytes; i++)
				{
					testbuffer[i] = buffer[i];
				}


				Open();

				if (isOpen)
				{
					core_SharedSetup(index);
					core_WriteEEPROM((UINT)index, &testbuffer);
				}
			}
			else
			{
				this->CastTextMessage("Filesize does not match target");
			}
		}
		else
		{
			this->CastTextMessage("ECU select index out of bounds");
		}
	}
	void noNameGizmo::TAP_ReadSram(int index)
	{
		if (index > 0 && index <= (int)core_NoTargets())
		{
			this->Open();

			if (isOpen)
			{
				core_SharedSetup(index);
				core_DumpSRAM((UINT)index);
			}
		}
		else
		{
			this->CastTextMessage("ECU select index out of bounds");
		}
	}
    array <BYTE>^ noNameGizmo::returnBufferBytes(int index, bool eeprom)
    {
		UINT noBytes = returnTargetSizeFLASH(index);

		if (eeprom)
		{
			noBytes = returnTargetSizeEEPROM(index);
		}

		if (!core_ReturnWorkStatus())
        {
            return nullptr;
        }

        array<BYTE>^ retArr = gcnew array <BYTE>(noBytes);

        BYTE *ptr = (BYTE *)core_ReturnBufferPointer();

        for (UINT i = 0; i < noBytes; i++)
            retArr[i] = *ptr++;

        return retArr;
    }

	array <BYTE>^ noNameGizmo::returnBufferBytesSRAM(int index)
	{
		UINT noBytes = returnTargetSizeSRAM(index);

		if (!core_ReturnWorkStatus())
		{
			return nullptr;
		}

		array<BYTE>^ retArr = gcnew array <BYTE>(noBytes);

		BYTE *ptr = (BYTE *)core_ReturnBufferPointer();

		for (UINT i = 0; i < noBytes; i++)
			retArr[i] = *ptr++;

		return retArr;
	}

	void noNameGizmo::Open()
	{
        this->Close();
        timeBeginPeriod(1);
        // this->write_lock->WaitOne();

		// TODO: Change these to something else just to prevent regular apps from sending crap to the adapter
		this->read_ep_id = ReadEndpointID::Ep01;
		this->write_ep_id = WriteEndpointID::Ep03;
		this->CastTextMessage("\n\n");
        this->CastTextMessage("Open sent");

		try
		{
			this->usb_finder = gcnew UsbDeviceFinder((UINT)0xFFFF, (UINT)0x0107);
			Debug::Assert(this->usb_finder != nullptr);

			this->default_dev = UsbDevice::OpenUsbDevice(this->usb_finder);
			if (!this->default_dev)
			{
                thisptr->CastTextMessage("Error: No adapter found");
				throw gcnew Exception("No adapter found");
			}

			WinUsbDevice^ winusb_dev = dynamic_cast<WinUsbDevice^>(this->default_dev);
			WinUsbDevice^ new_dev;
			if (!winusb_dev || !winusb_dev->GetAssociatedInterface(0, new_dev) || !new_dev)
			{
                thisptr->CastTextMessage("Error: Failed to claim");
				throw gcnew Exception("Failed to claim");
			}
			this->usb_dev = new_dev;

			this->ep_reader = this->usb_dev->OpenEndpointReader(this->read_ep_id);
			this->ep_writer = this->usb_dev->OpenEndpointWriter(this->write_ep_id);
			if (!this->ep_reader || !this->ep_writer)
			{
                thisptr->CastTextMessage("Error: Failed to open enpoints");
				throw gcnew Exception("Failed to open enpoints");
			}

			isOpen = true;
			this->ep_reader->ReadFlush();

			this->ep_reader->DataReceived += gcnew EventHandler<EndpointDataEventArgs^>
				(&noNameGizmo::readAdapter);

			this->ep_reader->DataReceivedEnabled = true;
		}

		catch (...)
		{
			this->Close();
			throw;
		}

		finally
		{
			// this->write_lock->ReleaseMutex();
            // this->write_lock->ReleaseMutex();
			// USB_TransferSize = 0;

		}
	}

	void noNameGizmo::writeAdapter(array<BYTE>^ buf, UINT timeout)
	{
		try
		{
			// thisptr->write_lock->WaitOne();

			Debug::Assert(thisptr->ep_writer != nullptr);
			int bytes_written;

			if (thisptr->ep_writer->Write(buf, timeout, bytes_written) != ErrorCode::Ok ||
				bytes_written != bytes_written)
			{
				throw gcnew Exception("Failed to write to adapter");
			}
		}

		finally
		{
            // thisptr->write_lock->ReleaseMutex();
		}
	}

    BYTE rxbuffer[4096];
	void noNameGizmo::storeToBuffer(array<BYTE>^ buf, int len)
	{
        for (int i = 0; i < len; i++)
			rxbuffer[i] = buf[i];

        core_HandleRecData(&rxbuffer, (UINT)len);
	}

	void noNameGizmo::readAdapter(Object^ sender, EndpointDataEventArgs^ e)
	{
        // thisptr->read_lock->WaitOne();
		Debug::Assert(e != nullptr && e->Buffer != nullptr);

		if (e->Count > 0)
			thisptr->noNameGizmo::storeToBuffer(e->Buffer, e->Count);

        // thisptr->read_lock->ReleaseMutex();
	}
};
