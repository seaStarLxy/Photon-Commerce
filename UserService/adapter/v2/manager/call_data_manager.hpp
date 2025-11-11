// Copyright (c) 2025 seaStarLxy.
// All Rights Reserved.

#pragma once
#include <type_traits> // for std::is_base_of
#include <vector>
#include <memory>
#include <UserService/v1/user_service.grpc.pb.h>
#include <grpcpp/completion_queue.h>
#include <boost/asio/io_context.hpp>


namespace user_service::adapter::v2 {
    // CallDataBase定义文件包含了该文件，这里使用前向声明更多是为了防止嵌套包含
    class CallDataBase;
    // 提供 manager 统一接口
    class ICallDataManager {
    public:
        ICallDataManager(const ssize_t initiate_size, const std::shared_ptr<boost::asio::io_context>& ioc, grpc::ServerCompletionQueue * cq):
            initial_size_(initiate_size), ioc_(ioc), cq_(cq) {
            if (!cq_) {
                throw std::invalid_argument("CQ cannot be null.");
            }
        }
        virtual ~ICallDataManager() = default;
        // 子类需要重写：用特定的 grpc服务 注册 call_data 到 CQ
        virtual void RegisterCallData(CallDataBase* call_data) = 0;
        // 子类需要重写：返回特定的业务逻辑指针
        virtual void* GetBusinessService() = 0;
        boost::asio::io_context& GetIOContext() const {
            return *ioc_;
        }
    protected:
        size_t initial_size_;
        std::shared_ptr<boost::asio::io_context> ioc_;
        grpc::ServerCompletionQueue *cq_;
    };

    // 每种特定类型的 CallData 对应一个 Manager
    template<typename GrpcServiceType, typename CallDataType, typename BusinessServiceType>
    class CallDataManager final: public ICallDataManager {
        static_assert(std::is_base_of_v<CallDataBase, CallDataType>, "CallDataType must derive from ICallData");
    public:
        CallDataManager(const size_t initial_size, GrpcServiceType* grpc_service, BusinessServiceType* business_service,
            const std::shared_ptr<boost::asio::io_context>& ioc, grpc::ServerCompletionQueue *cq)
            : ICallDataManager(initial_size, ioc, cq), grpc_service_(grpc_service), business_service_(business_service) {
            if (!grpc_service_) {
                throw std::invalid_argument("Service and CQ cannot be null.");
            }
        }

        ~CallDataManager() override = default;

        void Start() {
            for (size_t i = 0; i < initial_size_; ++i) {
                auto call_data = std::make_unique<CallDataType>(this);
                RegisterCallData(call_data.get());
                pool_.push_back(std::move(call_data));
            }
        }

        void RegisterCallData(CallDataBase* call_data) override {
            auto specific_call_data = static_cast<CallDataType*>(call_data);
            grpc_service_->RequestRegister(&specific_call_data->ctx_, &specific_call_data->request_, &specific_call_data->responder_, cq_, cq_, specific_call_data);
        }

        void* GetBusinessService() override {
            return business_service_;
        }

        GrpcServiceType * grpc_service_;
        BusinessServiceType * business_service_;
        std::vector<std::unique_ptr<CallDataType>> pool_;
    };
}
