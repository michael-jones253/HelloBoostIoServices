//
//  IoCircularBuffer.h
//  AsyncIo
//
//  Created by Michael Jones on 2/08/2015.
//  https://github.com/michael-jones253/HelloBoostIoServices
//

#ifndef __HelloAsio__IoCircularBuffer__
#define __HelloAsio__IoCircularBuffer__

#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <cstdint>

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

/// <summary>
/// Custom allocator for resizing/reserving a vector for circular buffer use.
/// </summary>
template <class T>
class ResizeReserveAllocator: public std::allocator<T>
{
public:
	ResizeReserveAllocator() {}
	template<class U>
	ResizeReserveAllocator(const ResizeReserveAllocator<U> &) {}
	template<class U>
	ResizeReserveAllocator(ResizeReserveAllocator<U> &&) {}

	// Needed as a minimum allocator on windows.
	template <class U>
	struct rebind
	{
		typedef ResizeReserveAllocator<U> other;
	};

    // This is needed for a reserve which shuffles data from one part of the container to another and requires
    // copying of this data.
    template<class U>
    void construct(U* me, T&& rhs) {
        // std::cout << "move construct" << std::endl;
        *me = std::move(rhs);
    };
    
    // This is needed for a resize without default initialisation wiping out data in the reserved/spare capacity area.
    template <class U, class... Args> void construct(U*, Args&&...) {
        // std::cout << "variadic construct" << std::endl;
    }

	// At least on OS X this does not get called by any vector re-allocation methods.
	// At least on windows this does not result in executable (can break point) code, I think the variadic
	// construct takes over.
	/*
	template <class U> void construct(U* me, const T&rhs) {
	std::cout << "copy construct" << std::endl;
	*me = rhs;
	}
	*/
};

namespace AsyncIo
{
    using IoAsyncReadSomeInCallback = std::function<void(uint8_t* bufPtr, size_t len, std::function<void(size_t)>&&handler)>;
    
    using IoNotifyAvailableCallback = std::function<void(size_t available)>;

	/// <summary>
	/// Circular buffer for receiving data from TCP or UDP.
	/// </summary>
    class IoCircularBuffer
	{
    private:
		std::atomic<bool> _hasContext;
		std::atomic<bool> _shouldRead;
        std::vector<uint8_t, ResizeReserveAllocator<uint8_t>> _buffer;
        int _chunkSize;
        IoAsyncReadSomeInCallback _readSomeIn;
        IoNotifyAvailableCallback _notifyAvailable;
    public:
        IoCircularBuffer(IoAsyncReadSomeInCallback readSome);
        IoCircularBuffer();
        
        IoCircularBuffer& operator=(IoCircularBuffer&& rhs);
        
		void BeginChainedRead(IoNotifyAvailableCallback&& notifySome, int chunkSize, bool immediate = true);

		// For when the above is called with immediate == false.
		void BeginReadSome() {
			if (_shouldRead) {
				return;
			}

			_shouldRead = true;
			ReadSome();
		}

		void EndReadSome() { _shouldRead = false;  }
        
        void Consume(size_t len);
        
        void CopyTo(std::vector<uint8_t>& dest, int len);
        
        uint8_t const* Get() const;
        
        size_t Size() const;
        
        // This is only needed for memory consumption diagnostics.
        size_t Capacity() const;

    private:
		void ReadSome();
		void ReadSomeHandler(size_t bytesRead);
    };
}
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

#endif /* defined(__HelloAsio__IoCircularBuffer__) */
