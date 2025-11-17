

```shell
# 前台运行 envoy
envoy -c /home/seastar/ECommerceSystem-Microservices/apiGateway/envoy.yaml -l info
```

```shell
# 生成 proto.pb
/home/seastar/ECommerceSystem-Microservices/UserService/cmake-build-debug/vcpkg_installed/x64-linux/tools/protobuf/protoc \
  -I /home/seastar/ECommerceSystem-Microservices/IDL \
  -I /home/seastar/ECommerceSystem-Microservices/third_party/googleapis \
  --include_imports \
  --descriptor_set_out=/home/seastar/ECommerceSystem-Microservices/apiGateway/proto.pb \
  /home/seastar/ECommerceSystem-Microservices/IDL/UserService/v1/user_service.proto
  

```