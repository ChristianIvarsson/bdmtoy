#pragma once

using namespace System;

namespace NameMeDLL
{
	public ref class textEventArgs : System::EventArgs
	{
	private:
		String ^_text;
	public:
		String ^infoText()
		{   return _text; }
		textEventArgs(String ^text)
		{   _text = text;          }
	};

    public ref class progressEventArgs : System::EventArgs
    {
    private:
        int _percentage;
    public:
        int percentage()
        {   return _percentage; }
        progressEventArgs(int percentage)
        {   _percentage = percentage;   }
    };

	// Forward text messages, progress bar and future CAN messages
	public ref class eventForwarder
	{
	protected:
		void CastTextMessage(String ^text);
        void CastProgress(int percentage);
	public:
		delegate void TextMessage(Object ^ sender, textEventArgs ^ E);
        delegate void ProgressUpdate(Object ^ sender, progressEventArgs ^ E);

        event TextMessage^ onTextMessage;
        event ProgressUpdate^ onProgress;
	};








}