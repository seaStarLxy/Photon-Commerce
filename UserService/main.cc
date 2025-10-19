// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#include <iostream>
#include "server/user_service_server.h"

using namespace UserService;

int main()
{
    std::cout << "Hello World!" << std::endl;
    UserServiceServer server;
    server.Run();
    return 0;
}
