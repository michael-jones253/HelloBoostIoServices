/*
*  IoLog.cpp
*  IoServices
*
*  Created by Michael Jones on 30/05/2016.
*  https://github.com/michael-jones253/HelloBoostIoServices
*
*/

#include "stdafx.h"
#include "IoLog.h"
#include "IoLogConsumer.h"

#include <chrono>

using namespace std;
using namespace std::chrono;

namespace AsyncIo
{
	IoLog::IoLog() :
		mStrStr{},
		mAvailable{}
	{
	}

	IoLog::IoLog(const IoLog& rhs) :
		mStrStr{ rhs.mStrStr.str() }
	{
		mAvailable = rhs.mAvailable;
		mAvailable = true;
	}

	IoLog::IoLog(IoLog&& rhs)
	{
        // Compiler bug fixed by gcc 5.0
		// mStrStr = move(rhs.mStrStr);
        mStrStr << rhs.mStrStr.str();
		mAvailable = rhs.mAvailable;
		rhs.mAvailable = false;
	}

	IoLog& IoLog::operator=(IoLog&& rhs)
	{
        // Compiler bug fixed by gcc 5.0
		// mStrStr = move(rhs.mStrStr);
        mStrStr << rhs.mStrStr.str();
		mAvailable = rhs.mAvailable;
		rhs.mAvailable = false;

		return *this;
	}

	/// <summary>
	/// Moves the log into the consumer upon destruction.
	/// </summary>
	IoLog::~IoLog()
	{
        try
        {
            if (mAvailable)
            {
                IoLogConsumer::Consume(move(*this));
            }
        }
        catch(const exception&)
        {
        }

		mAvailable = false;
	}

	std::wostream& operator << (std::wostream& os, const IoLog& rhs)
	{
		os << rhs.mStrStr.str();
		return os;
	}

	void IoLog::Finalise()
	{
		mAvailable = false;
	}
}
