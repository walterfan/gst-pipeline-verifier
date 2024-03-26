#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "pipeline_builder.h"

using namespace std;
using namespace testing;
using namespace hefei;


TEST(PipelineBuilderTest, build_progress)
{
    BuildProgress bp = BuildProgress::NOT_STARTED;
    ASSERT_TRUE(BuildProgress::LINKED_ELEMENTS > bp);
    bp = BuildProgress::LINKED_ELEMENTS;
    ASSERT_TRUE(BuildProgress::REMOVED_ELEMENTS > bp);
    ASSERT_TRUE(BuildProgress::UNLINKED_ELEMENTS > bp);
}
