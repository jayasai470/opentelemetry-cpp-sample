#include <iostream>
#include <future>

#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

#include "telemetry.h"

#include "opentelemetry/logs/provider.h"
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
    std::string metricsName{"starstream_prometheus"};
    std::string metricsPath{"0.0.0.0:8081"};
    telemetry::initMetrics(metricsName, metricsPath);

    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    HttpServer server;
    server.config.port = 8080;

    auto provider = metrics_api::Provider::GetMeterProvider();
    auto meter = provider->GetMeter("main_loop");
    auto counter = meter->CreateDoubleCounter("one_counter", OPENTELEMETRY_SDK_VERSION);
    const std::vector<std::pair<std::string, std::string>> labels = {
        {"key1", "value1"},
    };
    counter->Add(1.0, labels);

    //  GET-example for the path /info
    // Responds with request-information
    server.resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
    {
        auto provider = metrics_api::Provider::GetMeterProvider();
        auto meter = provider->GetMeter("http_rescource");
        auto counter = meter->CreateDoubleCounter("http_server_request_count", OPENTELEMETRY_SDK_VERSION);
        const std::vector<std::pair<std::string, std::string>> labels = {
            {"path", "/info"},
            {"method", "GET"}};
        counter->Add(1.0, labels);

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