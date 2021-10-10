# Unix Socket
So, I wanted to transfer some data from an Android app to another Android app on the same device. And no Java, only native code.
So I looked for [options](https://en.wikipedia.org/wiki/Inter-process_communication). Since Android is extremely annoying when it comes to file-system, preventing any process from seeing anything else but its own directory, any solution that depends on file-system are dismissed (like memory mapped files). I know it's supposedly done for security, bit this really limits our options. 
The straightforward options remaining are:
- Pipes
- Sockets.
- Unix domain sockets.
## Pipes
- One way communication. To have two-way communication, use two pipes.
- Come in two flavors, unnamed (anonymous, bound to the input/output streams of participating parties) or named (attached to the file-system => dismissed). [This article](https://en.wikipedia.org/wiki/Unix_domain_socket) mentions sending file descriptors over sockets to grant access that otherwise would be prohibited. I should try it someday.
- Anonymous pipes might work if I spawn one of the processes as the child of the other. Didn't try it, though.

## Sockets
- The communicating parties needn't be on the same machine, which is a plus. But when they are, it has a large overhead compared to other solutions.
- Tested and works nicely on Android, but I hoped for better performance.

## Unix Domain Sockets
- Are supposed to be about 7 times faster than regular network TCP sockets, according to [benchmarks](https://newbedev.com/tcp-loopback-connection-vs-unix-domain-socket-performance).
- Unix sockets are attached to the file system. They are affected by the file system permissions (and the Android jail).
- Can use an abstract namespace and avoid the file-system issues. This is what this sample is about. While it worked nicely on Linux, it didn't work on Android. [This SO question](https://stackoverflow.com/q/31068440/1942069) suggests it worked prior to Android 5.1. I tested it on Android 10, and indeed, couldn't use it for inter-process communication. It worked within the same process, but not across difference processes. Maybe we can send the file descriptors over regular sockets. I should try it later.

Anyway, enjoy this failed attempt!

## Building and Usage
To build,

	gcc socket.c -o socket.o

In one process (like, a terminal window), do:

	./socket.o -s

In Another one, do:

	./socket.o -c
## References
- [Sockets vs Unix Domain Sockets](https://lists.freebsd.org/pipermail/freebsd-performance/2005-February/001143.html)
- [Named pipes](https://en.wikipedia.org/wiki/Named_pipe)
- [Sockets](https://www.thegeekstuff.com/2011/12/c-socket-programming/)

