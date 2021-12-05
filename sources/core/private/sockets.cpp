
#include <core/sockets.h>
#include <core/diagnostic.h>
#include <type_traits>
#include <string.h> // for memcpy
#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib,"Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/udp.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <unistd.h> // for close
    #include <fcntl.h>
#endif

namespace djup
{
    namespace 
    {
        std::string GetLastErrorString()
        {
            #ifdef _WIN32
                const int error_code = WSAGetLastError();

                char * msg = nullptr;
                FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, error_code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&msg, 0, NULL);
                std::string result = msg;
                LocalFree(msg);

                return result;
            #else
                return strerror(errno);
            #endif
        }
    }

    Ip4Address::Ip4Address(const char * i_address)
    {
        struct in_addr address;
        #ifdef _WIN32
            address.S_un.S_addr = inet_addr(i_address);
        #else
            address.s_addr = inet_addr(i_address);
        #endif

        static_assert(sizeof(in_addr) == sizeof(m_data));
        memcpy(&m_data, &address, sizeof(in_addr));
    }

    Ip4Address::Ip4Address(Span<const unsigned char> i_internal_data)
    {
        assert(i_internal_data.size() == sizeof(m_data));
        memcpy(&m_data, i_internal_data.data(), sizeof(in_addr));
    }

    Udp4Socket::Udp4Socket(const ConstructionParams & i_params)
    {   
        #ifdef _WIN32

            static_assert(std::is_same_v<decltype(m_socket), SOCKET>);
            static_assert(s_invalid_socket == INVALID_SOCKET);

            static struct InitOnce
            {
                InitOnce()
                {
                    WSADATA wsa_data;
                    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
                        Error("WSAStartup failed: ", GetLastErrorString());
                }
            } init_once;

            m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(m_socket == INVALID_SOCKET)
                Error("Failed to create the socket: ", GetLastErrorString());

            struct sockaddr_in this_addr = {};
            this_addr.sin_family = AF_INET;
            this_addr.sin_port = htons(i_params.m_port);
            this_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            if(bind(m_socket, (struct sockaddr*)&this_addr, sizeof(this_addr)) != 0)
            {
                closesocket(m_socket);
                Error("Failed to bind the socket: ", GetLastErrorString());
            }

            int len = sizeof(this_addr);
            if(getsockname(m_socket, (struct sockaddr*)&this_addr, &len) != 0 ||
                len != sizeof(this_addr))
            {
                closesocket(m_socket);
                Error("Failed to get the address of the socket: ", GetLastErrorString());
            }

            m_port = ntohs(this_addr.sin_port);

            u_long non_blocking = i_params.m_blocking ? 0 : 1;
            if(ioctlsocket(m_socket, FIONBIO, &non_blocking) == SOCKET_ERROR)
            {
                closesocket(m_socket);
                Error("Failed to set the blocking mode: ", GetLastErrorString());
            }

            if(i_params.m_send_buffer_size != 0)
            {
                int buffer_size = static_cast<int>(i_params.m_send_buffer_size);
                if(static_cast<size_t>(buffer_size) != i_params.m_send_buffer_size)
                {
                    closesocket(m_socket);
                    Error("Send buffer size too high: ", i_params.m_send_buffer_size);
                }

                if(setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)) == -1)
                {
                    closesocket(m_socket);
                    Error("Failed to set send buffer size: ", GetLastErrorString());
                }
            }

            if(i_params.m_receive_buffer_size != 0)
            {
                int buffer_size = static_cast<int>(i_params.m_receive_buffer_size);
                if(static_cast<size_t>(buffer_size) != i_params.m_receive_buffer_size)
                {
                    closesocket(m_socket);
                    Error("Receive buffer size too high: ", i_params.m_receive_buffer_size);
                }

                if(setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)) == -1)
                {
                    closesocket(m_socket);
                    Error("Failed to set receive buffer size: ", GetLastErrorString());
                }
            }

        #else

            m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(m_socket == s_invalid_socket)
                Error("Failed to create the socket: ", GetLastErrorString());

            struct sockaddr_in this_addr = {};
            this_addr.sin_family = AF_INET;
            this_addr.sin_port = htons(i_params.m_port);
            this_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            if(bind(m_socket, (struct sockaddr*)&this_addr, sizeof(this_addr)) != 0)
            {
                close(m_socket);
                Error("Failed to bind the socket: ", GetLastErrorString());
            }

            socklen_t len = sizeof(this_addr);
            if(getsockname(m_socket, (struct sockaddr*)&this_addr, &len) != 0 ||
                len != sizeof(this_addr))
            {
                close(m_socket);
                Error("Failed to get the address of the socket: ", GetLastErrorString());
            }

            m_port = ntohs(this_addr.sin_port);

            int flags = fcntl(m_socket, F_GETFL, 0);
            if(flags == -1)
            {
                close(m_socket);
                Error("Failed to get the blocking mode: ", GetLastErrorString());
            }
            if(i_params.m_blocking)
                flags &= ~O_NONBLOCK;
            else
                flags |= O_NONBLOCK;
            if(fcntl(m_socket, F_SETFL, flags) != 0)
            {
                close(m_socket);
                Error("Failed to set the blocking mode: ", GetLastErrorString());
            }

            if(i_params.m_send_buffer_size != 0)
            {
                int buffer_size = static_cast<int>(i_params.m_send_buffer_size);
                if(static_cast<size_t>(buffer_size) != i_params.m_send_buffer_size)
                {
                    close(m_socket);
                    Error("Send buffer size too high: ", i_params.m_send_buffer_size);
                }

                if(setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)) == -1)
                {
                    close(m_socket);
                    Error("Failed to set send buffer size: ", GetLastErrorString());
                }
            }

            if(i_params.m_receive_buffer_size != 0)
            {
                int buffer_size = static_cast<int>(i_params.m_receive_buffer_size);
                if(static_cast<size_t>(buffer_size) != i_params.m_receive_buffer_size)
                {
                    close(m_socket);
                    Error("Receive buffer size too high: ", i_params.m_receive_buffer_size);
                }

                if(setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)) == -1)
                {
                    close(m_socket);
                    Error("Failed to set receive buffer size: ", GetLastErrorString());
                }
            }

        #endif
    }

    Udp4Socket::~Udp4Socket()
    {
        if(m_socket != s_invalid_socket)
        {
            #ifdef _WIN32
                shutdown(m_socket, SD_BOTH);
                closesocket(m_socket);
            #else
                shutdown(m_socket, SHUT_RDWR);
                close(m_socket);
            #endif
        }
    }

    void Udp4Socket::Send(Ip4Address i_dest_address, uint16_t i_dest_port, 
        Span<const unsigned char> i_data)
    {
        // Send on empty socket is undefined behaviour
        assert(m_socket != s_invalid_socket);
        
        struct sockaddr_in dest_addr = {};
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(i_dest_port);
        assert(sizeof(dest_addr.sin_addr.s_addr) == i_dest_address.GetInternalData().size());
        memcpy(&dest_addr.sin_addr.s_addr, i_dest_address.GetInternalData().data(), sizeof(dest_addr.sin_addr.s_addr));

        const int result = sendto(m_socket, 
            reinterpret_cast<const char*>(i_data.data()), 
            static_cast<int>(i_data.size()),
            0,
            (struct sockaddr*)&dest_addr,
            sizeof(dest_addr) );

        if(result == -1)
            Error("Send failed: ", GetLastErrorString());
        else if(result != static_cast<int>(i_data.size()))
            Error("Sent ", result, " bytes, should have sent ", i_data.size());
    }


    Udp4Socket::ReceiveResult Udp4Socket::Receive(Span<unsigned char> i_dest)
    {
        // Receive on empty socket is undefined behaviour
        assert(m_socket != s_invalid_socket);

        struct sockaddr_in recv_addr = {};
        recv_addr.sin_family = AF_INET;
        recv_addr.sin_port = htons(0);
        recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        #ifdef _WIN32
            int address_len = sizeof(recv_addr);
        #else
            socklen_t address_len = sizeof(recv_addr);
        #endif
        const int recv_result = recvfrom(m_socket, 
            reinterpret_cast<char*>(i_dest.data()), 
            static_cast<int>(i_dest.size()),
            0,
            (struct sockaddr*)&recv_addr, &address_len);

        ReceiveResult result;
        if(recv_result == -1)
        {
            #ifdef _WIN32
                const int error_code = WSAGetLastError();
                if(error_code == WSAEWOULDBLOCK)
                {
                    return result;
                }
                else if(error_code == WSAEMSGSIZE)
                {
                    result.m_more_data = true;
                    result.m_source_port = recv_addr.sin_port;
                    return result;
                }
                else
                    Error("Receive failed: ", GetLastErrorString());
            #else
                const int error_code = errno;
                if(error_code == EWOULDBLOCK || error_code == EAGAIN)
                {
                    return result;
                }
                else if(error_code == EMSGSIZE)
                {
                    result.m_more_data = true;
                    result.m_source_port = recv_addr.sin_port;
                    return result;
                }
                else
                    Error("Receive failed: ", GetLastErrorString());
            #endif
        }
        else if(recv_result == 0)
        {
            return result;
        }
        else
        {
            if(address_len != sizeof(recv_addr))
                Error("Receive: wromg source address size");

            assert(recv_result > 0);
            result.m_received_bytes = static_cast<size_t>(recv_result);
            result.m_source_port = recv_addr.sin_port;
            return result;
        }
    }
}
