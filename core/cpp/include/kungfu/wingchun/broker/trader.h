//
// Created by Keren Dong on 2019-06-20.
//

#ifndef WINGCHUN_TRADER_H
#define WINGCHUN_TRADER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::broker
{
    class Trader : public yijinjing::practice::apprentice
    {
    public:
        explicit Trader(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &source, const std::string &account_id);

        virtual ~Trader() = default;

        const std::string &get_account_id() const
        { return account_id_; }

        const std::string &get_source() const
        { return source_; }

        virtual const longfist::enums::AccountType get_account_type() const = 0;

        virtual bool insert_order(const event_ptr &event) = 0;

        virtual bool cancel_order(const event_ptr &event) = 0;

        virtual bool req_position() = 0;

        virtual bool req_account() = 0;

        virtual void on_start() override;

    protected:

        void publish_state(longfist::enums::BrokerState state)
        {
            auto s = static_cast<int32_t>(state);
            write_to(0, longfist::types::BrokerStateUpdate::tag, s);
        }

    private:
        std::string source_;
        std::string account_id_;
    };
}

#endif //WINGCHUN_TRADER_H