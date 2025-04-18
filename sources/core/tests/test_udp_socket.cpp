
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/sockets.h>
#include <core/diagnostic.h>
#include <future>
#include <thread>
#include <chrono>
#include <atomic>

namespace core
{
    namespace tests
    {
        void TestUdpSocket_Procedure(size_t i_thread_index, std::atomic_uint16_t io_ports[2],
            std::atomic_uint32_t & io_received_count)
        {
            Udp4Socket::ConstructionParams params;
            params.m_blocking = false;
            params.m_receive_buffer_size = 1024 * 1024;
            params.m_send_buffer_size = 1024 * 512;
            Udp4Socket socket(params);

            io_ports[i_thread_index] = socket.GetPort();

            bool this_received = false;

            while(io_received_count < 2)
            {
                const unsigned char message[] = "Hello!";

                int16_t other_port = io_ports[i_thread_index ^ 1].load();
                if(other_port != 0)
                    socket.Send(Ip4Address("127.0.0.1"), other_port, message);

                if(!this_received)
                {
                    unsigned char received_message[64];
                    if(auto received = socket.Receive(received_message))
                    {
                        if(received.m_more_data)
                            Error("Udp4 Test - Truncated datagram");
                        else if(received.m_received_bytes != std::size(message))
                            Error("Udp4 Test - Truncated datagram");
                        if(!Span(received_message, received.m_received_bytes).content_equals(Span(message)))
                            Error("Udp4 Test - Received wrong message");

                        io_received_count++;
                        this_received = true;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds{50});
            }
        }

        void TestUdpSocket()
        {
            Print("Test: Core - UdpSocket...");

            std::atomic_uint32_t received_count = 0;
            std::atomic_uint16_t ports[2] = {};

            auto future_1 = std::async([&received_count, &ports]{
                TestUdpSocket_Procedure(0, ports, received_count);
            });

            auto future_2 = std::async([&received_count, &ports]{
                TestUdpSocket_Procedure(1, ports, received_count);
            });

            future_1.wait();
            future_2.wait();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
