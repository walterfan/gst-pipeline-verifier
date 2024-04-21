#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <numeric>
#include <stdint.h>
#include "file_util.h"
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace testing;
using namespace hefei;

// using ::testing::Return;
// using ::testing::Mock;

TEST(FileUtilTest, read_pipeline)
{
    std::string yaml_file = "../etc/config.yaml";
    if (!file_exists(yaml_file))
    {
        yaml_file = "./etc/config.yaml";
    }
    YAML::Node rootNode = YAML::LoadFile(yaml_file);
    cout << "node type=" <<rootNode.Type() << endl;
    auto explainNode = rootNode["pipelines"];
    for (YAML::const_iterator it = explainNode.begin(); it != explainNode.end(); ++it)
    {
        std::cout << "key=" << it->first.as<std::string>() << ", value type="<< (int)it->second.Type() << "\n";
        auto pipelineNode = it->second;
        if (pipelineNode.Type() == YAML::NodeType::Map) {
            auto pipeline_tags = pipelineNode["tags"];
            auto pipeline_desc = pipelineNode["desc"];
            auto pipeline_steps = pipelineNode["steps"];
            std::cout << "tags=" << pipeline_tags.as<std::string>()
                      << ", desc=" << pipeline_desc.as<std::string>() << std::endl;

            auto vec = pipeline_steps.as<std::vector<std::string>>();
            std::string result = "";
            for (auto &step : vec)
            {
                result += step + ", ";
            }
            std::cout << "step count=" << vec.size() << ", result=" << result << endl;
        }
        if (pipelineNode.Type() == YAML::NodeType::Sequence)
        {
            auto vec = pipelineNode.as<std::vector<std::string>>();
            std::string result = "";
            for (auto &step : vec)
            {
                result += step + ", ";
            }
            std::cout << "step count=" << vec.size() << ", result=" << result << endl;
        }
    }
}
