#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "string_util.h"

using namespace std;
using namespace testing;
using namespace hefei;


TEST(StringUtilTest, startendswith)
{
	std::string str =  "'image/jpeg,width=1920,height=1080,framerate=20/1'";
    bool ret1 = startswith(str, "'");
    bool ret2 = endswith(str, "'");
    cout << "ret1=" << ret1 << ", ret2=" << ret2 << endl;
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);

}

TEST(StringUtilTest, SplitStringIntoPair)
{
    string input = "key=value";
    string separator = "=";
    pair<string, string> output;

    split(input, separator, output);

    EXPECT_EQ(output.first, "key");
    EXPECT_EQ(output.second, "value");
}

TEST(StringUtilTest, SplitEmptyString)
{
    string input = "";
    string separator = "=";
    pair<string, string> output;

    int result = split(input, separator, output);

    EXPECT_EQ(result, 0);
    EXPECT_TRUE(output.first.empty());
    EXPECT_TRUE(output.second.empty());
}

TEST(StringUtilTest, NoSeparator)
{
    string input = "noseparator";
    string separator = "=";
    pair<string, string> output;

    int result = split(input, separator, output);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(output.first, input);
    EXPECT_TRUE(output.second.empty());
}


TEST(SplitFunctionTest, HandlesSingleSeparator)
{
    std::map<std::string, std::string> resultMap;
    EXPECT_EQ(2, split("key1=value1;key2=value2", ";", "=", resultMap));
    EXPECT_EQ(1, resultMap.count("key1"));
    EXPECT_EQ("value1", resultMap["key1"]);
    EXPECT_EQ(1, resultMap.count("key2"));
    EXPECT_EQ("value2", resultMap["key2"]);
}

TEST(SplitFunctionTest, HandlesMultipleSeparators)
{
    std::map<std::string, std::string> resultMap;
    EXPECT_EQ(3, split("key1=value1,key2=value2,key3=value3", ",", "=", resultMap));
    EXPECT_EQ(1, resultMap.count("key1"));
    EXPECT_EQ("value1", resultMap["key1"]);
    EXPECT_EQ(1, resultMap.count("key2"));
    EXPECT_EQ("value2", resultMap["key2"]);
    EXPECT_EQ(1, resultMap.count("key3"));
    EXPECT_EQ("value3", resultMap["key3"]);
}

TEST(StringUtilTest, ReplaceVariables)
{
    string desc = "Hello ${name}, the value of ${value} is ${value}";
    map<string, string> var_map = {{"${name}", "John"}, {"${value}", "10"}};

    int num_replaced = replace_variables(desc, var_map);

    EXPECT_EQ(num_replaced, 3);
    EXPECT_EQ(desc, "Hello John, the value of 10 is 10");
}

TEST(StringUtilTest, ReplaceVariablesNoMatch)
{
    string desc = "No variables here";
    map<string, string> var_map = {{"name", "John"}};

    int num_replaced = replace_variables(desc, var_map);

    EXPECT_EQ(num_replaced, 0);
    EXPECT_EQ(desc, "No variables here");
}

TEST(StringUtilTest, ReplaceVariablesEmpty)
{
    string desc;
    map<string, string> var_map;

    int num_replaced = replace_variables(desc, var_map);

    EXPECT_EQ(num_replaced, 0);
    EXPECT_TRUE(desc.empty());
}
