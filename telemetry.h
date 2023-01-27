#pragma once

#include <string>

class telemetry
{
public:
    static void initMetrics(const std::string &name, const std::string &addr);
    static void cleanupMetrics();
    static void initLogger();
};