#include "stdafx.h"
#include "IoLogConsumer.h"

using namespace std;

namespace AsyncIo
{
	IoLogConsumerFn IoLogConsumer::_logConsumer = nullptr;

	void IoLogConsumer::AttachLogger(IoLogConsumerFn&& fn)
	{
		_logConsumer = move(fn);
	}

	void IoLogConsumer::Consume(IoLog&& ioLog)
	{
		if (_logConsumer != nullptr)
		{
			_logConsumer(move(ioLog));
		}
	}

	IoLog IoLogConsumer::DoLog()
	{
		return IoLog();
	}

}