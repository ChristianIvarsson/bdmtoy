#include "event.h"

using namespace System;

namespace NameMeDLL
{
	void eventForwarder::CastTextMessage(String ^text)
	{
		onTextMessage(this, gcnew textEventArgs(text));
	}
    void eventForwarder::CastProgress(int percentage)
    {
        onProgress(this, gcnew progressEventArgs(percentage));
    }
}