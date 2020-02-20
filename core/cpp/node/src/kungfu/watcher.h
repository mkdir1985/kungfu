//
// Created by Keren Dong on 2020/1/14.
//

#ifndef KUNGFU_NODE_WATCHER_H
#define KUNGFU_NODE_WATCHER_H

#include <napi.h>

#include <kungfu/yijinjing/practice/apprentice.h>
#include "operators.h"
#include "journal.h"

namespace kungfu::node
{
    class Watcher : public Napi::ObjectWrap<Watcher>, public yijinjing::practice::apprentice
    {
    public:
        explicit Watcher(const Napi::CallbackInfo &info);
        
        ~Watcher();

        Napi::Value GetLocation(const Napi::CallbackInfo &info);

        void SetLocation(const Napi::CallbackInfo &info, const Napi::Value &value)
        {}

        Napi::Value IsUsable(const Napi::CallbackInfo &info);

        Napi::Value IsLive(const Napi::CallbackInfo &info);

        Napi::Value IsStarted(const Napi::CallbackInfo &info);

        Napi::Value Setup(const Napi::CallbackInfo &info);

        Napi::Value Step(const Napi::CallbackInfo &info);

        Napi::Value GetState(const Napi::CallbackInfo &info);

        void SetState(const Napi::CallbackInfo &info, const Napi::Value &value);

        Napi::Value GetLedger(const Napi::CallbackInfo &info);

        void SetLedger(const Napi::CallbackInfo &info, const Napi::Value &value);

        Napi::Value PublishState(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

    protected:
        void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;
        void react() override;

    private:
        static Napi::FunctionReference constructor;
        yijinjing::data::location ledger_location_;
        Napi::ObjectReference state_ref_;
        Napi::ObjectReference ledger_ref_;
        serialize::JsUpdateState update_state;
        serialize::JsUpdateState update_ledger;
        serialize::JsPublishState publish;
    };
}


#endif //KUNGFU_NODE_WATCHER_H