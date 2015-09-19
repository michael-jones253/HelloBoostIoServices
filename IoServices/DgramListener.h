
#ifndef __HelloAsio__DgramListener__
#define __HelloAsio__DgramListener__

#include <iostream>
#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

namespace AsyncIo
{
	// Hide UDP listener.
	class UdpListener;
	class DgramListener;

	using DgramErrorCallback = std::function<void(const std::string& msg)>;

	using DgramReceiveCallback = std::function<void(std::shared_ptr<DgramListener>, int bytesAvailable)>;

	// MJ Review: - use this for TCP and put in a separate file.
	struct IoEndPoint
	{
		std::string IpAddress;
		int Port;
	};

	/// <summary>
	/// For logging and error reporting.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="groupType">The end point.</param>
	/// <returns>End point as readable text stream.</returns>
	inline std::ostream& operator<<(std::ostream &os, const IoEndPoint &endPoint)
	{
		os << endPoint.IpAddress << ":" << endPoint.Port;

		return os;
	}

	/// <summary>
	/// Hides the boost implementation of the UDP Listener from the application.
	/// NB this class is generated by IO services methods and not to be instantiated directly by application code.
	/// </summary>
	class DgramListener : public std::enable_shared_from_this<DgramListener>
	{
	private:
		std::weak_ptr<UdpListener> _udpListener;

	public:
		DgramListener(std::shared_ptr<UdpListener> udpListener);

		~DgramListener();

		/// <summary>
		/// Asynchronous write of a string message.
		/// NB if the length of the message is greater than MTU it will be split across datagrams.
		/// </summary>
		/// <param name="msg">The string message to send.</param>
		/// <param name="nullTerminate">Whether to null terminate the string or not.</param>
		void AsyncWrite(std::string&& msg, bool nullTerminate);

		/// <summary>
		/// Consume from the start of the circular buffer and copy to the destination buffer.
		/// </summary>
		/// <param name="buf">The destination buffer.</param>
		/// <param name="len">The length to be copied/extracted.</param>
		void ConsumeInto(uint8_t* buf, int len);

		/// <summary>
		/// Consume from the start of the circular buffer and copy to the back of the destination.
		/// </summary>
		/// <param name="dest">The destination vector.</param>
		/// <param name="len">Length to be consumed/copied.</param>
		void ConsumeTo(std::vector<uint8_t>& dest, int len);

		/// <summary>
		/// Returns a pointer to the start of circular buffer data.
		/// </summary>
		/// <returns>Pointer to received data.</returns>
		const uint8_t* Data() const;

		/// <summary>
		/// Returns the size of the available circular buffer data.
		/// </summary>
		/// <returns>Size of the received data.</returns>
		size_t Size() const;

		void Consume(int len);

		/// <summary>
		/// Stop listening.
		/// </summary>
		void StopListening();

		IoEndPoint GetPeerEndPoint() const;
	};
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__DgramListener__) */