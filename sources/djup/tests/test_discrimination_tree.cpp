
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/substitution_graph.h>
#include <private/pattern/pattern_info.h>
#include <tests/test_utils.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        void DiscriminationTree_()
        {
            Print("Test: djup - DiscriminationTree...");

            const std::filesystem::path artifact_path = GetArtifactPath("discrimination");

            // create or clean artifact path
            if (std::filesystem::exists(artifact_path))
            {
                for (auto file : std::filesystem::recursive_directory_iterator(artifact_path))
                    if (file.is_regular_file())
                        std::filesystem::remove(file.path());
            }
            else
            {
                std::filesystem::create_directories(artifact_path);
            }

            pattern::DiscriminationTree discrimination_net;
            discrimination_net.AddPattern(2, "a(b(c(real d)1))");
            static bool save_it = false;
            if (save_it)
            {
                discrimination_net.ToGraphWiz("discrimination").SaveAsImage(artifact_path / "discr.png");
            }

            pattern::DiscriminationTree discrimination_net1;
            discrimination_net1.AddPattern(2, "a(b(c(real d))1)");
            if (save_it)
            {
                discrimination_net1.ToGraphWiz("discrimination").SaveAsImage(artifact_path / "discr1.png");
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
