// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once

namespace UserService {
    class ICallData {
    public:
        virtual ~ICallData() = default;
        virtual void Proceed() = 0;
    };
}