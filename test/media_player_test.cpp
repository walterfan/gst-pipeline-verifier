#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include <nlohmann/json.hpp>
#include "media_player.h"

using json = nlohmann::json;

using namespace std;
using namespace testing;
using namespace hefei;

class MediaPlayerTest: public testing::Test
{
public:
	MediaPlayerTest() {
    }

    virtual ~MediaPlayerTest() { }

    virtual void SetUp() {
    	std::string strClassName(::testing::UnitTest::GetInstance()->current_test_info()->test_case_name());
    	std::string strFuncName(::testing::UnitTest::GetInstance()->current_test_info()->name());
    	m_strTestName = strClassName + "." + strFuncName;

        m_media_player = std::make_unique<MediaPlayer>();
    	printf("- %s begin \n", m_strTestName.c_str());

    }
    virtual void TearDown()  {
    	printf("- %s end \n", m_strTestName.c_str());

    }


protected:

    string m_strTestName;

    std::unique_ptr<MediaPlayer> m_media_player;
};

TEST_F(MediaPlayerTest, find_video_files) {
    std::string folder = "./material";
    int count = m_media_player->find_video_files(folder);
    printf("count=%d\n", count);
    ASSERT_GE(count, 5);
    auto info = m_media_player->to_string();
    printf("media_player: %s\n", info.c_str());
}

TEST_F(MediaPlayerTest, play_media_file) {
    MediaFile media_file;
    int ret = m_media_player->play_media_file(media_file);
    printf("ret=%d\n", ret);
    ASSERT_EQ(ret, 0);
}