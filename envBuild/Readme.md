
构建开发环境
docker build -t grpc-dev:v1 .
docker build -t grpc-dev:v2 .

docker run -it --rm --name temp-test -v ..:/app grpc-dev:v2 bash

构建运行环境
docker build -t customized-grpc:v1 .

docker-compose up -d

docker-compose down

> extra: 
curl
sudo apt install libboost-all-dev
> apt install libboost-coroutine-dev

> zip unzip tar
