

```bash
/usr/local/grpc_install/bin/protoc \
  -I /app/IDL \
  -I /app/third_party/googleapis \
  --include_imports \
  --descriptor_set_out=/app/apiGateway/proto.pb \
  /app/IDL/UserService/v1/user_service.proto
  
  
```