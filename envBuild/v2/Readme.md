# 基于 wsl 开发环境部署
```shell
# 额外安装部分
sudo apt install curl zip unzip tar build-essential gdb pkg-config bison flex autoconf
```

```shell
cd envBuild/v2
docker run -d \
  --name envoy-gateway-v2 \
  -p 8080:8080 \
  --restart unless-stopped \
  -v "$(pwd)/../../apiGateway/envoy.yaml":/etc/envoy/envoy.yaml \
  -v "$(pwd)/../../apiGateway/proto.pb":/etc/envoy/proto.pb \
  envoyproxy/envoy:contrib-v1.34.9
```

