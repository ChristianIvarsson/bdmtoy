#ifndef __HEADER_H
#define __HEADER_H


#pragma once
#include "../../../core/core.h"
#include "event.h"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Runtime;
using namespace System::Runtime::InteropServices;
using namespace LibUsbDotNet;
using namespace LibUsbDotNet::Main;
using namespace std;

typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

namespace NameMeDLL
{
	public ref class noNameGizmo : eventForwarder
	{
	private:

		static noNameGizmo  ^ thisptr;
		UsbDevice^ usb_dev;
		UsbDeviceFinder^ usb_finder;
        UsbDevice^ default_dev;
		UsbEndpointReader^ ep_reader;
		UsbEndpointWriter^ ep_writer;
        void core_SharedSetup(int index);

		bool isOpen;

        static Mutex^ write_lock = gcnew Mutex;
        static Mutex^ read_lock = gcnew Mutex;


	protected:
		ReadEndpointID read_ep_id;
		WriteEndpointID write_ep_id;

	public:

        static void managedProgress(int percentage);
		static void managedString(char* text);
        static void managedCallback();
		
		noNameGizmo();
		virtual ~noNameGizmo();

		static void writeAdapter(array<BYTE>^ buf, UINT timeout);
		static void readAdapter(Object^ sender, EndpointDataEventArgs^ e);

		void storeToBuffer(array<BYTE>^ buf, int len);

        String ^returnCoreVersion();
		const UINT returnNumberOfTargets();
		String ^returnTargetName(int index);
        String ^returnTargetInfo(int index);
		UINT returnTargetSizeFLASH(int index);
		UINT returnTargetSizeEEPROM(int index);
        UINT returnTargetSizeSRAM(int index);
        array <BYTE>^ returnBufferBytes(int index, bool eeprom);
		array <BYTE>^ returnBufferBytesSRAM(int index);

		void TAP_Dump(int index);
		void TAP_Flash(int index, array <BYTE>^ buffer);

		void TAP_ReadEeprom(int index);
		void TAP_WriteEeprom(int index, array <BYTE>^ buffer);

		void TAP_ReadSram(int index);

		virtual void Close();
		virtual void Open();

	};
}

#endif