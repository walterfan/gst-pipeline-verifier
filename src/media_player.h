#pragma once

#include <memory>
#include <vector>
#include <string>

namespace hefei {

enum class MediaCategory {
    AUDIO,
    VIDEO,
    APPLICATION
};

enum class MediaState {
   UNKNOWN,
   PLAYING,
   STOPPED
};


struct MediaFile {
    MediaCategory category = MediaCategory::VIDEO;
    MediaState state = MediaState::UNKNOWN;

    std::string name;
    std::string path;
    std::string desc;
    std::string tags;

};


class MediaPlayer {
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    int find_video_files(std::string& folder);
    /**
     * @brief
     *
     * gst-launch-1.0 filesrc location=./material/talk.mp4
     * ! decodebin name=dec
     * ! videoconvert
     * ! timeoverlay
     * ! autovideosink dec.
     * ! audioconvert
     * ! audioresample
     * ! autoaudiosink
     *
     * @param media_file 
     * @return int 
     */
    int play_media_file(MediaFile& media_file);
    std::string to_string();
private:
    MediaFile m_current_media_file;
    std::vector<MediaFile> m_media_files;

};

}// namespace hefei