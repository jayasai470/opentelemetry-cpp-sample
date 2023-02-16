#include <iostream>
#include <future>

#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

#include "telemetry.h"

#include "opentelemetry/sdk/version/version.h"
#include "opentelemetry/metrics/provider.h"

namespace nostd = opentelemetry::nostd;
namespace metrics_api = opentelemetry::metrics;

#include <Simple-Web-Server/server_http.hpp>

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;


int main()
{
    std::cout << "starting .....";
    std::string metricsName{"metrics_prometheus"};
    std::string metricsPath{"0.0.0.0:8081"};
    telemetry::initMetrics(metricsName, metricsPath);
    telemetry::initLogger();
    telemetry::initTracer();

    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    HttpServer server;
    server.config.port = 8080;

    //  GET-example for the path /info
    // Responds with request-information
    server.resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
    {
       auto counter = telemetry::getOrCreateCounter("http.server.request.count", "Total number of HTTP requests", "requests");
        counter->Add(1, {{"path", request->path},{"method", request->method}});
        
        auto span        = telemetry::getTracer()->StartSpan("health check span");
        auto ctx         = span->GetContext();
  
        auto logger = telemetry::getLogger();
        logger->EmitLogRecord(opentelemetry::logs::Severity::kDebug, "opentelemetry ...", ctx.trace_id(),
                        ctx.span_id(), ctx.trace_flags(),
                        opentelemetry::common::SystemTimestamp(std::chrono::system_clock::now()));
        sleep(2);
        // nested span
        opentelemetry::trace::StartSpanOptions spanOptions;
        spanOptions.parent = ctx;
        auto nested_span = telemetry::getTracer()->StartSpan("after logs", spanOptions);
        sleep(3);

        stringstream stream;
        stream << "<h1>Request from " << request->remote_endpoint().address().to_string() << ":" << request->remote_endpoint().port() << "</h1>";

        stream << request->method << " " << request->path << " HTTP/" << request->http_version;

        stream << "<h2>Query Fields</h2>";
        auto query_fields = request->parse_query_string();
        for (auto &field : query_fields)
            stream << field.first << ": " << field.second << "<br>";

        stream << "<h2>Header Fields</h2>";
        for (auto &field : request->header)
            stream << field.first << ": " << field.second << "<br>";

        response->write(stream);
        nested_span.get()->End();
    };

    server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/)
    {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    };

    // Start server and receive assigned port when server is listening for requests
    promise<unsigned short> server_port;
    thread server_thread([&server, &server_port]()
                         {
    // Start server
    server.start([&server_port](unsigned short port) {
      server_port.set_value(port);
    }); });
    cout << "Server listening on port " << server_port.get_future().get() << endl
         << endl;

    server_thread.join();
}