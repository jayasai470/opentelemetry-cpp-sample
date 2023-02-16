#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <random>

#include <chrono>

#include "opentelemetry/sdk/trace/id_generator.h"
#include "opentelemetry/sdk/common/exporter_utils.h"
#include "opentelemetry/sdk/trace/random_id_generator.h"

#include "xray_id_generator.h"

namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_api = opentelemetry::trace;

uint64_t gen_random()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

opentelemetry::trace::SpanId XrayIdGenerator::GenerateSpanId() noexcept
{
    uint8_t span_id_buf[trace_api::SpanId::kSize];
    // TODO random buffer
    return opentelemetry::trace::SpanId(span_id_buf);
}

/**
 * A trace_id consists of three numbers separated by hyphens. For example, 1-58406520-a006649127e371903a2de979. This includes:
 * The version number, that is, 1.
 * The time of the original request, in Unix epoch time, in 8 hexadecimal digits.
 * For example, 10:00AM December 1st, 2016 PST in epoch time is 1480615200 seconds, or 58406520 in hexadecimal digits.
 * A 96-bit identifier for the trace, globally unique, in 24 hexadecimal digits.
 */
opentelemetry::trace::TraceId XrayIdGenerator::GenerateTraceId() noexcept
{
    // refer https://github.com/alfianabdi/opentelemetry-cpp/blob/main/sdk/src/trace/aws_xray_id_generator.cc#L23
    uint8_t trace_id_buf[trace_api::TraceId::kSize];
    const auto p1 = std::chrono::system_clock::now();
    uint64_t ts = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
    uint64_t lo_rand = gen_random();
    uint64_t hi_rand = gen_random() & 0x00000000FFFFFFFF;
    uint64_t hi = ts << 32 | hi_rand;
    memcpy(&trace_id_buf[0], &lo_rand, sizeof(uint64_t));
    memcpy(&trace_id_buf[sizeof(uint64_t)], &hi, sizeof(uint64_t));
    for (size_t l = 0, h = trace_api::TraceId::kSize - 1; l < h; l++, h--)
    {
        std::swap(trace_id_buf[l], trace_id_buf[h]);
    }
    return trace_api::TraceId(trace_id_buf);
}
