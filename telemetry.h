#pragma once

#include <string>

#ifndef ENABLE_LOGS_PREVIEW
#define ENABLE_LOGS_PREVIEW
#endif

#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/trace/provider.h"

namespace nostd            = opentelemetry::nostd;
namespace logs             = opentelemetry::logs;
namespace metrics_api      = opentelemetry::metrics;
namespace trace            = opentelemetry::trace;

class telemetry {
public:
  static void initMetrics(const std::string &name, const std::string &addr);
  static void cleanupMetrics();
  static nostd::shared_ptr<metrics_api::Counter<uint64_t>> getOrCreateCounter(nostd::string_view name, nostd::string_view description, nostd::string_view unit);
  static void initLogger();
  static nostd::shared_ptr<logs::Logger> getLogger();
  static void initTracer();
  static nostd::shared_ptr<trace::Tracer> getTracer();
};