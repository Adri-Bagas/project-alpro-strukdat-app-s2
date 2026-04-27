#pragma once

#include <vlcpp/vlc.hpp>
#include <slint.h>
#include <string>
#include <memory>
#include "app-window.h"

// Declaration of the media metadata parser function
void parse_media_vlcpp(VLC::Instance& instance, const std::string& filepath, std::shared_ptr<slint::VectorModel<MediaItem>> ui_model);
