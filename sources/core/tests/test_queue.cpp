
#include <core/queue.h>
#include <core/diagnostic.h>
#include <vector>

namespace djup
{
    namespace tests
    {
        void TestQueue_Overflow(Queue<int, uint8_t> & i_queue)
        {
            for(int i = 0; i < 200; i++)
            {
                i_queue.Push(i);
            }
            for(int i = 0; i < 200; i++)
            {
                i_queue.Pop();
            }

            std::vector<uint8_t> handles;
            for(int i = 0; i < 100; i++)
            {
                handles.push_back(i_queue.Push(i));
            }

            for(size_t i = 0; i < handles.size(); i++)
            {
                int * found = i_queue.GetByHandle(handles[i]);
                SNABB_EXPECTS_EQ(*found, static_cast<int>(i));
            }
        }

        void TestQueue()
        {
            Print("Test: Core - Queue...");

            Queue<int, uint8_t> queue;

            for(int i = 0; i < 10; i++)
            {
                auto handle = queue.Push(i);
                SNABB_EXPECTS_EQ(handle, i + 1);
            }

            SNABB_EXPECTS_EQ(queue.GetAndPop(), 0);
            SNABB_EXPECTS_EQ(queue.GetAndPop(), 1);
            SNABB_EXPECTS_EQ(queue.GetAndPop(), 2);
            SNABB_EXPECTS_EQ(queue.GetAndPop(), 3);

            SNABB_EXPECTS_EQ(queue.GetByHandle(1), nullptr);
            SNABB_EXPECTS_EQ(queue.GetByHandle(2), nullptr);
            SNABB_EXPECTS_EQ(queue.GetByHandle(3), nullptr);
            SNABB_EXPECTS_EQ(queue.GetByHandle(4), nullptr);
            SNABB_EXPECTS_EQ(*queue.GetByHandle(5), 4);
            SNABB_EXPECTS_EQ(*queue.GetByHandle(6), 5);
            SNABB_EXPECTS_EQ(*queue.GetByHandle(7), 6);
            SNABB_EXPECTS_EQ(*queue.GetByHandle(8), 7);

            int expected = 4;
            for(int i : queue)
            {
                SNABB_EXPECTS_EQ(i, expected);
                expected++;
            }

            // TestQueue_Overflow(queue);
            
            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
