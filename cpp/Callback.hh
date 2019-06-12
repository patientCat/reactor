#pragma once

#include <functional>

typedef std::function<void(int fd, int events)> EventCallback;
