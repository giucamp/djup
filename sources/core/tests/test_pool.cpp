
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
        namespace
        {
            struct TestClass
            {
                int64_t m_value;

                static int64_t s_instances;

                TestClass(int64_t i_value) : m_value(i_value) 
                { 
                    ++s_instances; 
                }
                TestClass(const TestClass & i_source) : m_value(i_source.m_value)
                {
                    ++s_instances; 
                }
                TestClass(TestClass && i_source) noexcept : m_value(i_source.m_value)
                { 
                    ++s_instances; 
                }
                ~TestClass() 
                {
                    CORE_EXPECTS(s_instances > 0);
                    --s_instances; 
                }
            };

            int64_t TestClass::s_instances = 0;

            template <typename UINT>
                struct MirrorEntry
            {
                typename Pool<TestClass, UINT>::Handle m_handle;
                int64_t m_value;
            };

            void Pool_VersionTest()
            {
                using Handle = Pool<int>::Handle;

                Pool<int, size_t> pool(10);
                std::vector<Handle> handles;

                CORE_EXPECTS(!pool.IsValid(Handle{}));

                CORE_EXPECTS_EQ(pool.TryGetObject(Handle{}), nullptr);

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

            void Pool_VersionTest_2()
            {
                Pool<int> pool;

                Pool<int>::Handle handle_1, handle_2, handle_3, handle_4;

                handle_1 = pool.New();
                CORE_EXPECTS_EQ(handle_1.m_version, 0u);
                CORE_EXPECTS(pool.IsValid(handle_1));
                pool.Delete(handle_1);

                handle_2 = pool.New();
                CORE_EXPECTS_EQ(handle_2.m_version, 1u);
                CORE_EXPECTS(pool.IsValid(handle_2));
                pool.Delete(handle_2);

                handle_3 = pool.New();
                CORE_EXPECTS_EQ(handle_3.m_version, 2u);
                CORE_EXPECTS(pool.IsValid(handle_3));
                pool.Delete(handle_3);

                handle_4 = pool.New();
                CORE_EXPECTS_EQ(handle_4.m_version, 3u);                

                CORE_EXPECTS(!pool.IsValid(handle_1));
                CORE_EXPECTS(!pool.IsValid(handle_2));
                CORE_EXPECTS(!pool.IsValid(handle_3));
                CORE_EXPECTS(pool.IsValid(handle_4));

                // leave the pool empty
                pool.Delete(handle_4);
            }

            template <typename POOL, typename MIRROR_VECTOR>
                void TestIterator(POOL & i_pool, const MIRROR_VECTOR & i_mirror_pool)
            {
                using UInt = typename POOL::UInt;

                // create a multiset with all values
                std::unordered_multiset<int64_t> multiset;
                multiset.reserve(i_mirror_pool.size());
                for (auto & obj : i_mirror_pool)
                    multiset.insert(obj.m_value);

                // find and remove all values from the multiset
                UInt found_object = 0;
                for (auto & obj : i_pool)
                {
                    ++found_object;

                    auto it = multiset.find(obj.m_value);
                    CORE_EXPECTS(it != multiset.end());
                    multiset.erase(it);
                }
                CORE_EXPECTS_EQ(i_pool.GetObjectCount(), found_object);

                CORE_EXPECTS(multiset.empty());
            }

            template <typename UINT>
                void PoolUsageTest()
            {
                using Handle = typename Pool<TestClass, UINT>::Handle;
                using UInt = UINT;

                /* Test loop. Objects are randomly added and removed to both a pool
                   and a mirror vector. Every object has a 64-bit value that must
                   match in the container.
                   Add is more probable than remove, so the pool, will grow. */
                Pool<TestClass, UInt> pool;
                std::vector<MirrorEntry<UInt>> mirror_pool;

                std::random_device rand_dev;
                std::random_device::result_type seed = rand_dev();
                Print("(", sizeof(UINT)*8, " bits, seed: ", seed, ") ");
                std::mt19937 mt(seed);
                std::uniform_int_distribution<int> rnd_action(0, 10);
                std::uniform_int_distribution<int64_t> rnd_value(0, 10000);

                const int test_length = 1000'000;
                const size_t max_count_in_pool = 100;
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
                    case 2:
                    case 3:
                        if (pool.GetObjectCount() < max_count_in_pool)
                        {
                            const int64_t value = rnd_value(mt);
                            const Handle handle = pool.New(value);
                            mirror_pool.push_back({ handle, value });
                            CORE_EXPECTS(handle != Handle{});
                        }
                        break;

                    // delete an object
                    case 4:
                    case 5:
                    case 6:
                        if (!mirror_pool.empty())
                        {
                            const size_t mirror_index = std::uniform_int_distribution<size_t>
                                (0, mirror_pool.size() - 1)(mt);
                            Handle handle = mirror_pool[mirror_index].m_handle;
                            TestClass& obj = pool.GetObject(handle);

                            CORE_EXPECTS(pool.IsValid(handle));
                            CORE_EXPECTS(pool.TryGetObject(handle) == &obj);
                            CORE_EXPECTS_EQ(pool.GetObject(handle).m_value, obj.m_value);

                            mirror_pool.erase(mirror_pool.begin() + mirror_index);
                            pool.Delete(handle);
                            CORE_EXPECTS(!pool.IsValid(handle));
                            CORE_EXPECTS(pool.TryGetObject(handle) == nullptr);
                        }
                        break;

                    // check for handle validity
                    case 7:
                    {                        
                        std::vector<bool> visited;
                        visited.resize(mirror_pool.size());
                        for (size_t j = 0; j < mirror_pool.size(); ++j)
                        {
                            const MirrorEntry<UInt> & entry = mirror_pool[j];
                            CORE_EXPECTS(pool.IsValid(entry.m_handle));
                            visited[j] = true;
                        }
                        for (size_t j = 0; j < mirror_pool.size(); ++j)
                        {
                            if (!visited[j])
                            {
                                const MirrorEntry<UInt> & entry = mirror_pool[j];
                                CORE_EXPECTS(!pool.IsValid(entry.m_handle));
                            }
                        }
                    }
                    break;

                    // check for duplicate handles
                    case 8:
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
                    case 9:
                    {
                        CORE_EXPECTS(TestClass::s_instances >= 0);
                        const size_t count_in_pool = pool.GetObjectCount();
                        CORE_EXPECTS_EQ(count_in_pool, mirror_pool.size());
                        CORE_EXPECTS_EQ(count_in_pool,
                            NumericCast<size_t>(TestClass::s_instances));
                    }
                    break;

                    case 10:
                        TestIterator(pool, mirror_pool);
                        TestIterator(std::as_const(pool), mirror_pool);
                        break;

                    default:
                        break;
                    }
                }

                // empty the pool before destruction (it's a requirement of the class)
                for (const MirrorEntry<UInt>& entry : mirror_pool)
                {
                    TestClass& obj = pool.GetObject(entry.m_handle);

                    CORE_EXPECTS(pool.IsValid(entry.m_handle));
                    CORE_EXPECTS_EQ(pool.GetObject(entry.m_handle).m_value, obj.m_value);

                    pool.Delete(entry.m_handle);
                }

                Print("\b\b\b100%, ");
            }
        }

        void Pool_()
        {
            Print("Test: Core - Pool...");

            using Handle = Pool<int>::Handle;
            CORE_EXPECTS(Handle{} == Handle{});

            Pool_VersionTest();
            Pool_VersionTest_2();
            PoolUsageTest<size_t>();
            PoolUsageTest<uint32_t>();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
