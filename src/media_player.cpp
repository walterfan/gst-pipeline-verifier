#include "media_player.h"
#include "file_util.h"
#include <sstream>

namespace hefei {

MediaPlayer::MediaPlayer() {

}

MediaPlayer::~MediaPlayer() {

}

int MediaPlayer::find_video_files(std::string& folder) {
    std::vector<std::string> files;

    int count = retrieve_files(folder, ".mp4", files);
    if (count > 0) {
        for(auto& file: files) {
            MediaFile media_file;
            media_file.name = file;
            media_file.path = folder + "/" + file;
            m_media_files.push_back(media_file);
        }
    }

    return count;
}

int MediaPlayer::play_media_file(MediaFile& media_file) {
    return 0;
}

std::string MediaPlayer::to_string() {
    std::stringstream ss;
    for(auto& media_file: m_media_files) {
        ss << "name=" << media_file.name << ",";
        ss << "path=" << media_file.path << ",";
        ss << "state=" << (int)media_file.state << ",";
        ss << "category=" << (int)media_file.category << ",";
        ss << "\n ";
    }
    return ss.str();
}

}// namespace hefei