# HelloBoostIoServices
Boost asio based thread pool, timer and async IO services. This code has had extensive unit testing on windows and is currently in industrial use. The thread pool is interesting, but the network functionality has proved to be the most useful. This project achieves:
1. Hides all boost headers from application code using hidden implementations (pointer to impl).
2. Is platform independent - being maintained on both Windows and OS X (therefore should port to Linux).
3. Allows applications to rapidly put together TCP client and server network connections and UDP sending/receiving with all object lifetime taken care of.
4. Completely non-blocking API leveraging off Boost's async IO.

## Aim of project
I am maintaining some code which needs these things. It is based on boost, but has two problems:
1. Does not obey the requirement of async_write as specificied in the boost documentation: "The program must ensure that the stream performs no other write operations (such as async_write, the stream's async_write_some function, or any other composed operations that perform writes) until this operation completes."
2. Implements a circular buffer in an inefficient manner: uses a std::list with each item in the list containing just a single character. Which means each incoming byte from the socket is pushed one at a time. Bytes from the front of the list also have to be popped one at a time.

### Initial circular buffer approach attempted
Boost provides a circular buffer, so this was the obvious choice. I started writing something based on the boost circular buffer, but quickly discovered that it was not suited to adding chunks of bytes on the back and removing chunks from the front - it provides an interface for adding one item at a time which in this case is a byte. I would need to call linearize to flatten it out for transfer to another buffer or string and I was having difficulty finding out how to get reserve space and provide a pointer to that space into which a socket could write.

### The circular buffer implemented
I used vector for the storage because of the methods it provides convenient methods to add and remove data and it guarantees storage to be liniear. I had previously implemented a circular buffer here https://github.com/michael-jones253/AMQP-CPP-xcode-example/blob/master/MyAMQP/MyAMQPCircularBuffer.h This implementation kept its own pointer for where in the memory allocation remaining unconsumed data started and had to do pointer arithmentic. I decided to use vector's methods for resizing/reserving and erasing from the front to look after the pointers and pointer arithmentic.

### Implementation issues encountered
Resizing the buffer wiped out all data that had been read from the socket into the reserved area, so I had to implement my own custom allocator. The initial solution I found on stack overflow avoided the default construction of a byte, but did not cope with the vector reserve method which shuffled data. This was rectified with a modification to my allocator.

### Test driven design complexity issues
Test driven design is good for the following things:
1. Better testing (obviously).
2. Encourages coder to limit dependencies to other modules by producing components which can be independently tested or have a generic interface to other components it depends on.
3. Saves development time by being able to set up test situations more easily than the integrated components.

In this situation it did achieve the above, but the end result was harder to understand than if I had tightly coupled boost's callbacks with the buffer. I found it hard at times to understand the chain of callbacks I had created.
