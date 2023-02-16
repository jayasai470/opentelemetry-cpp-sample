#pragma once

#include "opentelemetry/sdk/trace/id_generator.h"

namespace trace_sdk = opentelemetry::sdk::trace;

class XrayIdGenerator : public trace_sdk::IdGenerator
{
    public:
        opentelemetry::trace::SpanId GenerateSpanId() noexcept override;
        opentelemetry::trace::TraceId GenerateTraceId() noexcept override;
};