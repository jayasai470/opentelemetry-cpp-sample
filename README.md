# opentelemetry-cpp-sample

sample repository to verify the issue on opentelemetry-cpp-sdk and metrics

## prerequisite
Install opentelemetry and prometheus cpp libs in your local

## steps to build and run
```
mkdir build && cd build
cmake .. && make
./otel-cpp-sample
```

## steps to reproduce
Hit info endpoint twice, and check the prometheus metrics endpoint
```
curl localhost:8080/info
curl localhost:8080/info
```

```
curl localhost:8081/metrics
```