/*
*  IoLogConsumer.h
*  IoServices
*
*  Created by Michael Jones on 30/05/2016.
*  https://github.com/michael-jones253/HelloBoostIoServices
*
*/
#ifndef IoLogConsumer_
#define IoLogConsumer_

#include "IoLog.h"

#include <functional>

#define LOG() IoLogConsumer::DoLog()

namespace AsyncIo
{
	using IoLogConsumerFn = std::function<void(IoLog&& ioLog)>;

	class IoLogConsumer
	{
	private:
		static IoLogConsumerFn _logConsumer;

	public:
		static void AttachLogger(IoLogConsumerFn&& fn);

		static void Consume(IoLog&& ioLog);

		static IoLog DoLog();
	};
}

#endif