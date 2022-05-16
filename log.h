#ifndef LOG_H
#define LOG_H

#include <fmt/ostream.h>
#undef FMT_HEADER_ONLY
#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif
#include "spdlog/spdlog.h"

#define LOGGER_NAME "SGCS"

#define LOG spdlog::get(LOGGER_NAME)

#endif // LOG_H
