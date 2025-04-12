
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/pool.h>
#include <core/diagnostic.h>
#include <core/numeric_cast.h>
#include <unordered_set>
#include <string>
#include <random>

namespace core
{
    namespace tests
    {
        void Pool_1()
        {
            Pool<std::string> pool(10);
            for (int i = 0; i < 15; i++)
                pool.New("string");
        }

        namespace
        {
            struct TestClass
            {
                int64_t m_value;

                static int64_t s_instances;

                TestClass(int64_t i_value) : m_value(i_value) { ++s_instances; }
                TestClass(int64_t i_value, double) : m_value(i_value) { ++s_instances; }
                TestClass(const TestClass & i_source) : m_value(i_source.m_value) { ++s_instances; }
                TestClass(TestClass && i_source) noexcept : m_value(i_source.m_value) { ++s_instances; }
                ~TestClass() 
                {
                    CORE_EXPECTS(s_instances >= 0);
                    --s_instances; 
                }
            };

            int64_t TestClass::s_instances = 0;

            void Pool_VersionTest()
            {
                using Handle = Pool<int>::Handle;
                
                Pool<int, size_t> pool(10);
                std::vector<Handle> handles;

                // fill
                for (int i = 0; i < 10; ++i)
                {
                    Handle handle = pool.New(i);
                    handles.push_back(handle);
                    CORE_EXPECTS_EQ(handle.m_version, 0u);
                }
                CORE_EXPECTS_EQ(pool.GetObjectCount(), 10u);

                // empty
                for (int i = 0; i < 10; ++i)
                {
                    pool.Delete(handles[i]);
                }
                handles.clear();

                // fill again
                for (int i = 0; i < 10; ++i)
                {
                    Handle handle = pool.New(i);
                    handles.push_back(handle);
                    CORE_EXPECTS_EQ(handle.m_version, 1u);
                }

                // empty again
                for (int i = 9; i >= 0; --i)
                {
                    Handle handle = handles[i];
                    CORE_EXPECTS(pool.IsValid(handle));
                    pool.Delete(handles[i]);
                }
            }

            template <typename UINT>
                void PoolUsageTest()
            {
                using Handle = typename Pool<TestClass, UINT>::Handle;
                using UInt = UINT;

                struct MirrorEntry
                {
                    typename Pool<TestClass, UInt>::Handle m_handle;
                    int64_t m_value;
                };

                /* Test loop. Objects are randomly added and removed to both a pool
                   and a mirror vector. Every object has a 64-bit value that must
                   match in the container.
                   Add is twice more probable than remove, so the pool, will grow. */
                Pool<TestClass, UInt> pool;
                std::vector<MirrorEntry> mirror_pool;

                std::random_device rand_dev;
                std::random_device::result_type seed = rand_dev();
                Print("(", sizeof(UINT)*8, " bits, seed: ", seed, ") ");
                std::mt19937 mt(seed);
                std::uniform_int_distribution<int> rnd_action(0, 5);
                std::uniform_int_distribution<int64_t> rnd_value(0, 10000);

                const int test_length = 100'000;
                const size_t max_count_in_pool = 1000;
                Print(" 0%");

                for (int i = 0; i < test_length; ++i)
                {
                    // print progress
                    if (i % 0x10 == 0)
                    {
                        int perc = i * 100 / test_length;
                        if (perc > 100)
                            perc = 100;
                        Print("\b\b\b");
                        if (perc < 10)
                            Print(" ", perc);
                        else
                            Print(perc);
                        Print('%');
                    }

                    const int action = rnd_action(mt);
                    switch (action)
                    {

                    // add an object
                    case 0:
                    case 1:
                        if (pool.GetObjectCount() < max_count_in_pool)
                        {
                            const int64_t value = rnd_value(mt);
                            const Handle handle = pool.New(value);
                            mirror_pool.push_back({ handle, value });
                        }
                        break;

                    // delete an object
                    case 2:
                        if (!mirror_pool.empty())
                        {
                            const size_t mirror_index = std::uniform_int_distribution<size_t>
                                (0, mirror_pool.size() - 1)(mt);
                            Handle handle = mirror_pool[mirror_index].m_handle;
                            TestClass& obj = pool.GetObject(handle);

                            CORE_EXPECTS(pool.IsValid(handle));
                            CORE_EXPECTS_EQ(pool.GetObject(handle).m_value, obj.m_value);

                            mirror_pool.erase(mirror_pool.begin() + mirror_index);
                            pool.Delete(handle);
                            CORE_EXPECTS(!pool.IsValid(handle));
                        }
                        break;

                    // check for handle validity
                    case 3:
                    {                        
                        std::vector<bool> visited;
                        visited.resize(mirror_pool.size());
                        for (size_t j = 0; j < mirror_pool.size(); ++j)
                        {
                            const MirrorEntry& entry = mirror_pool[j];
                            CORE_EXPECTS(pool.IsValid(entry.m_handle));
                            visited[j] = true;
                        }
                        for (size_t j = 0; j < mirror_pool.size(); ++j)
                        {
                            if (!visited[j])
                            {
                                const MirrorEntry& entry = mirror_pool[j];
                                CORE_EXPECTS(!pool.IsValid(entry.m_handle));
                            }
                        }
                    }
                    break;

                    // check for duplicate handles
                    case 4:
                    {
                        std::unordered_set<UInt> visited;
                        visited.reserve(mirror_pool.size());
                        for (size_t j = 0; j < mirror_pool.size(); ++j)
                        {
                            auto it = visited.insert(mirror_pool[j].m_handle.m_index);
                            CORE_EXPECTS(it.second);
                        }
                    }
                    break;

                    // check counts
                    case 5:
                    {
                        CORE_EXPECTS(TestClass::s_instances >= 0);
                        const size_t count_in_pool = pool.GetObjectCount();
                        CORE_EXPECTS_EQ(count_in_pool, mirror_pool.size());
                        CORE_EXPECTS_EQ(count_in_pool,
                            NumericCast<size_t>(TestClass::s_instances));
                    }
                    break;

                    default:
                        break;
                    }
                }

                // empty the pool before destruction (it's a requirement of the class)
                for (const MirrorEntry& entry : mirror_pool)
                {
                    TestClass& obj = pool.GetObject(entry.m_handle);

                    CORE_EXPECTS(pool.IsValid(entry.m_handle));
                    CORE_EXPECTS_EQ(pool.GetObject(entry.m_handle).m_value, obj.m_value);

                    pool.Delete(entry.m_handle);
                }

                PrintLn("\b\b\b");
            }
        }

        void Pool_()
        {
            Print("Test: Core - Pool...");

            Pool_VersionTest();
            PoolUsageTest<size_t>();
            PoolUsageTest<uint32_t>();

            PrintLn("successful");
        }
        


    } // namespace tests

} // namespace core
