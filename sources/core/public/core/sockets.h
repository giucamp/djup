
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/span.h>
#include <stdint.h>
#include <utility> // for std::swap

namespace djup
{
    class Ip4Address
    {
    public:

        Ip4Address() : m_data{} {}

        Ip4Address(const char * i_address);

        Ip4Address(Span<const unsigned char> i_internal_data);

        Span<const unsigned char> GetInternalData() const noexcept { return m_data; }

    private:
        alignas(void*) unsigned char m_data[4];
    };

    class Udp4Socket
    {
    public:

        struct ConstructionParams
        {
            /**< If the port is zero a value is chosen by the OS, and it can be retrieved by GetPort() */
            uint16_t m_port = 0;

            /**< Wether send and recive operations should be blocking */
            bool m_blocking = true;

            /**< Buffer size for sends, in bytes. If zero the OS-specific default is left. */
            size_t m_send_buffer_size = 0;

            /**< Buffer size for receives, in bytes. If zero the OS-specific default is left. */
            size_t m_receive_buffer_size = 0;
        };

        /**< Constructs an empty socket. Any communication operation except on an
            empty socket causes undefined behaviour. */
        Udp4Socket() = default;

        /**< Constructs a socket ready to be used for communication. */
        Udp4Socket(const ConstructionParams & i_params);

        ~Udp4Socket();

        Udp4Socket(Udp4Socket && i_source) noexcept
        {
            swap(*this, i_source);
        }

        Udp4Socket & operator = (Udp4Socket && i_source) noexcept
        {
            swap(*this, i_source);
            return *this;
        }

        friend void swap(Udp4Socket & i_first, Udp4Socket & i_second) noexcept
        {
            std::swap(i_first.m_socket, i_second.m_socket);
            std::swap(i_first.m_port, i_second.m_port);
        }

        uint16_t GetPort() const noexcept { return m_port; }

        /**< Sends a datagram to the destination port and address */
        void Send(Ip4Address i_dest_address, uint16_t i_dest_port,
            Span<const unsigned char> i_data);

        struct ReceiveResult
        {
            size_t m_received_bytes = 0; /**< Size of the datagram (number of bytes received) */

            Ip4Address m_source_address; /**< The ip address of the sender */

            uint16_t m_source_port = 0; /**< The port used by the sender */

            bool m_more_data = false; /**< if true the datagram was larger than the buffer,
                and it was truncated. The remaining content of the datagram is discarded, and
                m_received_bytes is set to zero. */

            /** Returns true wether a datagram has been received */
            explicit operator bool () const noexcept { return m_received_bytes > 0; }
        };

        /**< Tries to receive a datagram. This function blocks if the socket is blocking */
        ReceiveResult Receive(Span<unsigned char> i_dest);

    private:
        #ifdef _WIN32
            static const uintptr_t s_invalid_socket = (uintptr_t)~0;
            uintptr_t m_socket = s_invalid_socket;
        #else
            static const int s_invalid_socket = -1;
            int m_socket = s_invalid_socket;
        #endif
        uint16_t m_port = 0;
    };
}
