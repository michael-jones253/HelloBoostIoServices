/*
*  IoLog.h
*  IoServices
*
*  Created by Michael Jones on 30/05/2016.
*  https://github.com/michael-jones253/HelloBoostIoServices
*
*/

#ifndef IoLog_
#define IoLog_

#include <sstream>
#include <ostream>

namespace AsyncIo
{
	/// <summary>
	/// Created by the logging macros to provide an anonymous object that enqueues itself onto the logger upon destruction.
	/// </summary>
	class IoLog
	{
	private:
		std::wstringstream mStrStr;
		bool mAvailable;

	public:
		IoLog();
		IoLog(IoLog&& rhs);
		IoLog& operator=(IoLog&& rhs);
		IoLog(const IoLog&);
		IoLog& operator=(const IoLog&) = delete;

		/// <summary>
		/// Enqueues onto the logger upon destruction.
		/// </summary>
		~IoLog();


		// For building up a log message.
		template <class T>
		IoLog& operator<<(const T& thing)
		{
			mStrStr << thing;
			mAvailable = true;
			return *this;
		}

		// define an operator<< to take in std::endl. The above template function doesn't handle it.
		IoLog& operator<<(std::wostream& (*os)(std::wostream&))
		{
			mStrStr << os;
			mAvailable = true;
			return *this;
		}

		// define an operator<< to take in std::string. The above template function doesn't handle it.
		IoLog& operator<<(const std::string& str)
		{
			mStrStr << std::wstring(str.begin(), str.end());
			mAvailable = true;
			return *this;
		}

		// For the final write to logfile.
		friend std::wostream& operator << (std::wostream& os, const IoLog& rhs);

		/// <summary>
		/// Prevent further destruction of this logit from causing a call to enqueue.
		/// </summary>
		void Finalise();
	};

}
#endif