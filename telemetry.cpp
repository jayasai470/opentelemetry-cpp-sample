#include <map>
#include <memory>
#include <thread>
#include <unordered_map>

#include "telemetry.h"

// #include "opentelemetry/exporters/otlp/otlp_grpc_exporter.h"
// #include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
// #include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h"

#include "opentelemetry/exporters/prometheus/exporter.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"

#include "opentelemetry/exporters/ostream/log_record_exporter.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/logger_provider.h"

#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/sdk/trace/samplers/always_on.h"

#include "xray_id_generator.h"

namespace nostd = opentelemetry::nostd;
namespace common = opentelemetry::common;

namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace metrics_exporter = opentelemetry::exporter::metrics;
namespace metrics_api = opentelemetry::metrics;

namespace logs_exporter = opentelemetry::exporter::logs;
namespace sdk_logs = opentelemetry::sdk::logs;
namespace logs = opentelemetry::logs;

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_exporter = opentelemetry::exporter::trace;

// namespace otlp = opentelemetry::exporter::otlp;

nostd::shared_ptr<metrics_api::MeterProvider> provider;
nostd::shared_ptr<metrics_api::Meter> meter;
// store all counter data
std::unordered_map<nostd::string_view, nostd::shared_ptr<metrics_api::Counter<uint64_t>>> counter_data;

void telemetry::initMetrics(const std::string &name, const std::string &addr)
{
  metrics_exporter::PrometheusExporterOptions opts;
  if (!addr.empty())
  {
    opts.url = addr;
  }

  // initialize exporter
  std::shared_ptr<metrics_sdk::MetricReader> prometheus_exporter(new metrics_exporter::PrometheusExporter(opts));

  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};

  auto globalProvider = std::shared_ptr<metrics_api::MeterProvider>(new metrics_sdk::MeterProvider);
  // set global meter provider
  metrics_api::Provider::SetMeterProvider(globalProvider);

  auto p = std::static_pointer_cast<metrics_sdk::MeterProvider>(globalProvider);
  p->AddMetricReader(prometheus_exporter);

  // counter view
  std::string counter_name = name + "_counter";
  std::unique_ptr<metrics_sdk::InstrumentSelector> instrument_selector{
      new metrics_sdk::InstrumentSelector(metrics_sdk::InstrumentType::kCounter, counter_name)};
  std::unique_ptr<metrics_sdk::MeterSelector> meter_selector{
      new metrics_sdk::MeterSelector(name, version, schema)};
  std::unique_ptr<metrics_sdk::View> sum_view{
      new metrics_sdk::View{name, "description", metrics_sdk::AggregationType::kSum}};
  p->AddView(std::move(instrument_selector), std::move(meter_selector), std::move(sum_view));

  // histogram view
  std::string histogram_name = name + "_histogram";
  std::unique_ptr<metrics_sdk::InstrumentSelector> histogram_instrument_selector{
      new metrics_sdk::InstrumentSelector(metrics_sdk::InstrumentType::kHistogram, histogram_name)};
  std::unique_ptr<metrics_sdk::MeterSelector> histogram_meter_selector{
      new metrics_sdk::MeterSelector(name, version, schema)};
  std::unique_ptr<metrics_sdk::View> histogram_view{
      new metrics_sdk::View{name, "description", metrics_sdk::AggregationType::kHistogram}};
  p->AddView(std::move(histogram_instrument_selector), std::move(histogram_meter_selector),
             std::move(histogram_view));

  provider = metrics_api::Provider::GetMeterProvider();
  meter = provider->GetMeter(name);
}

void telemetry::cleanupMetrics()
{
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
}

nostd::shared_ptr<metrics_api::Counter<uint64_t>> telemetry::getOrCreateCounter(nostd::string_view name, nostd::string_view description, nostd::string_view unit)
{
  auto [it, result] = counter_data.try_emplace(name, nullptr);
  if (result)
  {
    auto _counter = meter->CreateUInt64Counter(name, description, unit);
    it->second = std::move(_counter);
    return it->second;
  }
  return it->second;
}

void telemetry::initLogger()
{
  // otlp grpc logger
  // opentelemetry::exporter::otlp::OtlpGrpcExporterOptions otlpOpts;
  // auto exporter = otlp::OtlpGrpcLogRecordExporterFactory::Create(otlpOpts);

  // auto sdkProvider = std::shared_ptr<sdk_logs::LoggerProvider>(new sdk_logs::LoggerProvider());
  // sdkProvider->AddProcessor(std::unique_ptr<sdk_logs::LogRecordProcessor>(new sdk_logs::SimpleLogRecordProcessor(std::move(exporter))));
  // auto apiProvider = nostd::shared_ptr<logs::LoggerProvider>(sdkProvider);
  // auto provider = nostd::shared_ptr<logs::LoggerProvider>(apiProvider);
  // logs::Provider::SetLoggerProvider(provider);
}

nostd::shared_ptr<logs::Logger> telemetry::getLogger()
{
  const std::string schema_url{"https://opentelemetry.io/schemas/1.11.0"};
  auto logger = logs::Provider::GetLoggerProvider()->GetLogger("Logger", "opentelelemtry_library", OPENTELEMETRY_SDK_VERSION, schema_url);
  return logger;
}

void telemetry::initTracer()
{
  // opentelemetry::exporter::otlp::OtlpGrpcExporterOptions otlpOpts;
  // auto exporter = otlp::OtlpGrpcExporterFactory::Create(otlpOpts);
  // auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));

  // // TODO: create resources
  // auto resource = opentelemetry::sdk::resource::Resource::Create({{"service_name", "http service"}});
  // auto alwaysOnSampler = std::unique_ptr<trace_sdk::Sampler>(new opentelemetry::sdk::trace::AlwaysOnSampler());
  // auto xrayIdGenerator = std::unique_ptr<trace_sdk::IdGenerator>(new XrayIdGenerator());

  // std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
  //     trace_sdk::TracerProviderFactory::Create(std::move(processor), resource, std::move(alwaysOnSampler), std::move(xrayIdGenerator));

  // // Set the global trace provider
  // trace_api::Provider::SetTracerProvider(provider);
}

nostd::shared_ptr<trace_api::Tracer> telemetry::getTracer()
{
  auto provider = trace_api::Provider::GetTracerProvider();
  return provider->GetTracer("starstream", OPENTELEMETRY_SDK_VERSION);
}