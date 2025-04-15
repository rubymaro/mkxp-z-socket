require 'socket'

tcp_socket = Socket.new(Socket::AF_INET, Socket::SOCK_STREAM, 0)
tcp_socket.setsockopt(Socket::IPPROTO_TCP, Socket::TCP_NODELAY, true)
sockaddr_server = Socket.sockaddr_in(9000, "124.61.178.91")

begin
    tcp_socket.connect(sockaddr_server)
#rescue Errno::ENETDOWN => e # the network is down
#rescue Errno::EADDRINUSE => e # the socket's local address is already in use
#rescue Errno::EINTR => e # the socket was cancelled
#rescue Errno::EINPROGRESS => e # a blocking socket is in progress or the service provider is still processing a callback function. Or a nonblocking connect call is in progress on the socket.
#rescue Errno::EALREADY => e # see Errno::EINVAL
#rescue Errno::EADDRNOTAVAIL => e # the remote address is not a valid address, such as ADDR_ANY TODO check ADDRANY TO INADDR_ANY
#rescue Errno::EAFNOSUPPORT => e # addresses in the specified family cannot be used with with this socket
rescue Errno::ECONNREFUSED => e # the target sockaddr was not listening for connections refused the connection request
    print(e.inspect << "\n\n" << e.backtrace.join("\n"))
#rescue Errno::EFAULT => e # the socket's internal address or address length parameter is too small or is not a valid part of the user space address
#rescue Errno::EINVAL => e # the socket is a listening socket
#rescue Errno::EISCONN => e # the socket is already connected
#rescue Errno::ENETUNREACH => e # the network cannot be reached from this host at this time
#rescue Errno::EHOSTUNREACH => e # no route to the network is present
#rescue Errno::ENOBUFS => e # no buffer space is available
#rescue Errno::ENOTSOCK => e # the socket argument does not refer to a socket
#rescue Errno::ETIMEDOUT => e # the attempt to connect timed out before a connection was made.
#rescue Errno::EWOULDBLOCK => e # the socket is marked as nonblocking and the connection cannot be completed immediately
#rescue Errno::EACCES => e # the attempt to connect the datagram socket to the broadcast address failed
end

$count = 0

loop do
    begin
        tcp_socket.write_nonblock("안녕 client #{$count}")
        #tcp_socket.write_nonblock("안녕 client #{$count}".encode("utf-8"))
        #tcp_socket.write_nonblock("안녕 client #{$count}".encode("utf-16"))
    rescue IO::WaitReadable => e # the socket is marked as nonblocking and the connection cannot be completed immediately
        print("WaitReadable")
    rescue Errno::ECONNRESET => e
        print(e.inspect)
        break
    end
    sleep 1
    $count += 1
end

tcp_socket.close

#p tcp_socket 