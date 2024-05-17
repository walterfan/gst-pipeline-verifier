#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;
using namespace testing;

TEST(JsonUtilTest, parse_json)
{
	json message;
    message["name"] = "alice";
    message["age"] = 100;

    std::string name = "name";

    bool ret = message.contains(name);
    ASSERT_TRUE(ret);

    ret = message.contains("sex");
    ASSERT_FALSE(ret);

    std::string parsed_name = message["name"].get<std::string>();
    ASSERT_EQ("alice", parsed_name);

    try {
        std::string parsed_sex = message["sex"].get<std::string>();
    }  catch (const nlohmann::json::parse_error &e) {
        cerr << "JSON parse error: " << e.what() << endl;
    }
    catch (const std::exception &e) {
        cerr << "Standard exception: " << e.what() << endl;
    }
}
