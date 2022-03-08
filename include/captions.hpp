//
// Created by quontas on 3/7/22.
//

#ifndef COG_GROUP_CONVO_CPP_CAPTIONS_HPP
#define COG_GROUP_CONVO_CPP_CAPTIONS_HPP

#include <string>
#include <mutex>
#include "cog-flatbuffer-definitions/caption_message_generated.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>

class CaptionModel {
private:
    std::string current_speech;
    std::string wrapped_speech;
    cog::Juror current_speaker;
    std::mutex text_mutex;
    bool clear_on_speaker_change = false;
    static const int LINE_WIDTH = 50;
    static const int SPACE_WIDTH = 1;

    void wrap_text(const std::string &text) {
        std::istringstream iss(text);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>{}};
        int space_left = LINE_WIDTH;
        std::vector<std::string> wrapped;
        for (const auto &token: tokens) {
            if (wrapped.empty()) {
                wrapped.push_back(token + " ");
                continue;
            }
            if (token.length() + SPACE_WIDTH > space_left) {
                wrapped.push_back(token + " ");
                space_left = LINE_WIDTH - (int) token.length();
            } else {
                wrapped.back() += token + " ";
                space_left -= (int) token.length() + SPACE_WIDTH;
            }
        }
        if (wrapped.size() == 1) {
            wrapped_speech = wrapped.front();
        } else {
            wrapped_speech = wrapped.rbegin()[1] + " " + wrapped.rbegin()[0];
        }
    }

public:
    [[maybe_unused]] CaptionModel(bool clear_on_speaker_change) {
        this->clear_on_speaker_change = clear_on_speaker_change;
    }

    void add_word(const std::string &new_word, cog::Juror speaker) {
        text_mutex.lock();
        if (current_speaker != speaker && clear_on_speaker_change) {
            current_speech.clear();
            current_speaker = speaker;
        }
        current_speech += new_word + " ";
        wrap_text(current_speech);
        text_mutex.unlock();
    }

    std::tuple<std::string, cog::Juror> get_current_text() {
        text_mutex.lock();
        auto tuple = std::make_tuple(wrapped_speech, current_speaker);
        text_mutex.unlock();
        return tuple;
    }
};

cog::Juror juror_from_string(const std::string &juror_str) {
    cog::Juror juror;
    if (juror_str == "juror-a") {
        juror = cog::Juror_JurorA;
    } else if (juror_str == "juror-b") {
        juror = cog::Juror_JurorB;
    } else if (juror_str == "juror-c") {
        juror = cog::Juror_JurorC;
    } else if (juror_str == "jury-foreman") {
        juror = cog::Juror_JuryForeman;
    } else {
        std::cerr << "Unknown speaker ID encountered: " << juror_str << std::endl;
        throw;
    }
    return juror;
}

void play_captions(nlohmann::json *caption_json, CaptionModel *model) {
    for (auto i = 0; i < caption_json->size(); ++i) {
        auto text = caption_json->at(i)["text"].get<std::string>();
        double delay;
        if (i == 0) {
            delay = caption_json->at(i)["delay"].get<double>();
        } else {
            delay = caption_json->at(i)["delay"].get<double>() - caption_json->at(i - 1)["delay"].get<double>();
        }
        auto speaker_id_str = caption_json->at(i)["speaker_id"].get<std::string>();
        auto speaker_id = juror_from_string(speaker_id_str);
        std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1000>>(delay));
        model->add_word(text, speaker_id);
    }
}

#endif //COG_GROUP_CONVO_CPP_CAPTIONS_HPP
