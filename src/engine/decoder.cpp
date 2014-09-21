 /* fre:ac - free audio converter
  * Copyright (C) 2001-2014 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <engine/decoder.h>

#include <smooth/io/drivers/driver_zero.h>

using namespace smooth::IO;

using namespace BoCA;
using namespace BoCA::AS;

Array<Threads::Mutex *>	 BonkEnc::Decoder::mutexes;
Threads::Mutex		 BonkEnc::Decoder::managementMutex;

BonkEnc::Decoder::Decoder()
{
	f_in	     = NIL;
	filter_in    = NIL;

	sampleOffset = 0;
}

BonkEnc::Decoder::~Decoder()
{
	Destroy();
}

Bool BonkEnc::Decoder::Create(const String &nFileName, const Track &track)
{
	static DriverZero	 zero_in;

	Registry	&boca = Registry::Get();

	if (nFileName.StartsWith("device://")) f_in = new InStream(STREAM_DRIVER, &zero_in);
	else				       f_in = new InStream(STREAM_FILE, nFileName, IS_READ);

	f_in->SetPackageSize(track.length >= 0 ? 196608 : 12288);

	if (f_in->GetLastError() != IO_ERROR_OK)
	{
		BoCA::Utilities::ErrorMessage("Cannot access input file: %1", nFileName);

		delete f_in;
		f_in = NIL;

		return False;
	}

	/* Create decoder component.
	 */
	filter_in = boca.CreateDecoderForStream(nFileName);

	if (filter_in == NIL)
	{
		BoCA::Utilities::ErrorMessage("Cannot create decoder component for input file: %1", nFileName);

		delete f_in;
		f_in = NIL;

		return False;
	}

	/* Lock decoder if it's not thread safe.
	 */
	if (!filter_in->IsThreadSafe())
	{
		managementMutex.Lock();

		if (mutexes.Get(filter_in->GetID().ComputeCRC32()) == NIL) mutexes.Add(new Threads::Mutex(), filter_in->GetID().ComputeCRC32());

		managementMutex.Release();

		mutexes.Get(filter_in->GetID().ComputeCRC32())->Lock();
	}

	/* Add decoder to stream.
	 */
	Track	 trackInfo = track;

	trackInfo.origFilename = nFileName;

	filter_in->SetAudioTrackInfo(trackInfo);

	if (f_in->AddFilter(filter_in) == False)
	{
		BoCA::Utilities::ErrorMessage("Cannot set up decoder for input file: %1\n\nError: %2", File(nFileName).GetFileName(), filter_in->GetErrorString());

		if (!filter_in->IsThreadSafe()) mutexes.Get(filter_in->GetID().ComputeCRC32())->Release();

		boca.DeleteComponent(filter_in);

		filter_in = NIL;

		delete f_in;
		f_in = NIL;

		return False;
	}

	/* Seek to sampleOffset if necessary.
	 */
	if (track.sampleOffset > 0 && !filter_in->Seek(track.sampleOffset))
	{
		Buffer<UnsignedByte>	 buffer(1024);
		Int64			 bytesLeft = track.sampleOffset * track.GetFormat().channels * (track.GetFormat().bits / 8);

		while (bytesLeft) bytesLeft -= Read(buffer, Math::Min((Int64) buffer.Size(), bytesLeft));
	}

	fileName     = nFileName;
	sampleOffset = track.sampleOffset;

	return True;
}

Bool BonkEnc::Decoder::Destroy()
{
	if (filter_in == NIL || f_in == NIL) return False;

	Registry	&boca = Registry::Get();

	f_in->RemoveFilter(filter_in);

	if (filter_in->GetErrorState()) BoCA::Utilities::ErrorMessage("Error: %1", filter_in->GetErrorString());

	if (!filter_in->IsThreadSafe()) mutexes.Get(filter_in->GetID().ComputeCRC32())->Release();

	boca.DeleteComponent(filter_in);

	delete f_in;

	filter_in    = NIL;
	f_in	     = NIL;

	fileName     = NIL;
	sampleOffset = 0;

	return True;
}

Bool BonkEnc::Decoder::GetStreamInfo(Track &track)
{
	return filter_in->GetStreamInfo(fileName, track);
}

Int BonkEnc::Decoder::Read(Buffer<UnsignedByte> &buffer, Int bytes)
{
	return f_in->InputData(buffer, bytes);
}

Bool BonkEnc::Decoder::Seek(Int64 sample)
{
	return filter_in->Seek(sampleOffset + sample);
}

Int64 BonkEnc::Decoder::GetInBytes() const
{
	return filter_in->GetInBytes();
}

String BonkEnc::Decoder::GetDecoderName() const
{
	return filter_in->GetName();
}

Void BonkEnc::Decoder::FreeLockObjects()
{
	foreach (Threads::Mutex *mutex, mutexes) delete mutex;

	mutexes.RemoveAll();
}
