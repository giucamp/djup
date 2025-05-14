
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/m2o_pattern/m2o_discrimination_tree.h>
#include <private/m2o_pattern/m2o_substitution_graph.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <tests/test_utils.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        void M2oDiscriminationTree_()
        {
            Print("Test: djup - m2o DiscriminationTree...");

            const std::filesystem::path artifact_path = GetArtifactPath("m2o_discrimination");

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

            m2o_pattern::DiscriminationTree discrimination_net;
            discrimination_net.AddPattern(2, "a(b(c(real d)1))");
            static bool save_it = false;
            if (save_it)
            {
                discrimination_net.ToGraphWiz("discrimination").SaveAsImage(artifact_path / "discr.png");
            }

            m2o_pattern::DiscriminationTree discrimination_net1;
            discrimination_net1.AddPattern(2, "a(b(c(real d))1)");
            if (save_it)
            {
                discrimination_net1.ToGraphWiz("discrimination").SaveAsImage(artifact_path / "discr1.png");
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
