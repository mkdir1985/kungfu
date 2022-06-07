//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_COMMON_H
#define WINGCHUN_COMMON_H

#include <cmath>
#include <locale>

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/time.h>

#define REGION_CN "CN"
#define REGION_HK "HK"

#define EXCHANGE_SSE "SSE"
#define EXCHANGE_SZE "SZE"
#define EXCHANGE_BSE "BSE"
#define EXCHANGE_SHFE "SHFE"
#define EXCHANGE_DCE "DCE"
#define EXCHANGE_CZCE "CZCE"
#define EXCHANGE_CFFEX "CFFEX"
#define EXCHANGE_INE "INE"
#define EXCHANGE_BINANCE "BINANCE"
#define EXCHANGE_HB "HB"

// 全市场exchange id定义
#define EXCHANGE_HK "HK"           // 港股: 4（香港交易所）
#define EXCHANGE_HK_OPTION "HKOPT" // 港股期货: 5（香港交易所）
#define EXCHANGE_US "US"           // 美股: 29（纳斯达克交易所）
#define EXCHANGE_GLFX "GLFX"       // 全球外汇: 41
#define EXCHANGE_IPE "IPE"         // LME\IPE: 45(LME（伦敦金属交易所）、ICE)
#define EXCHANGE_CBOT "CBOT"       // ES-CBOT: 62
#define EXCHANGE_CEC "CEC"         // ES-CEC: 63
#define EXCHANGE_LIFE "LIFE"       // ES-LIFE: 64
#define EXCHANGE_MTIF "MTIF"       // ES-MTIF: 65
#define EXCHANGE_NYCE "NYCE"       // ES-NYCE: 66、49
#define EXCHANGE_CMX "CMX"         // ES-CMX: 67
#define EXCHANGE_NYME "NYME"       // ES-NYME: 68
#define EXCHANGE_SIME "SIME"       // ES-SIME: 69
#define EXCHANGE_CME "CME"         // ES-CME: 70
#define EXCHANGE_IMM "IMM"         // ES-IMM: 71
#define EXCHANGE_WIDX "WIDX"       // ES-WIDX: 72
#define EXCHANGE_FREX "FREX"       // ES-FREX: 73
#define EXCHANGE_METL "METL"       // ES-METL: 74
#define EXCHANGE_IPM "IPM"         // 国际贵金属: 5000

#define SOURCE_SIM "sim"
#define SOURCE_CTP "ctp"
#define SOURCE_XTP "xtp"
#define SOURCE_BAC "barrich"

#define EPSILON (1e-6)
#define DOUBLEMAX (1e16) // 一亿亿, 2018年A股总市值不到50万亿

namespace kungfu::wingchun {
constexpr int64_t VOLUME_ZERO = 0;

// 商品类型根据exchange_id，映射对应的secType
static const std::unordered_map<std::string, longfist::enums::InstrumentType> g_exchange_id_type_map_ = {
    {EXCHANGE_HK, longfist::enums::InstrumentType::Stock},
    {EXCHANGE_HK_OPTION, longfist::enums::InstrumentType::StockOption},
    {EXCHANGE_US, longfist::enums::InstrumentType::Stock},
    {EXCHANGE_GLFX, longfist::enums::InstrumentType::Stock},
    {EXCHANGE_IPE, longfist::enums::InstrumentType::Future},
    {EXCHANGE_CBOT, longfist::enums::InstrumentType::Future},
    {EXCHANGE_CEC, longfist::enums::InstrumentType::Future},
    {EXCHANGE_LIFE, longfist::enums::InstrumentType::Future},
    {EXCHANGE_MTIF, longfist::enums::InstrumentType::Future},
    {EXCHANGE_NYCE, longfist::enums::InstrumentType::Future},
    {EXCHANGE_CMX, longfist::enums::InstrumentType::Future},
    {EXCHANGE_NYME, longfist::enums::InstrumentType::Future},
    {EXCHANGE_SIME, longfist::enums::InstrumentType::Future},
    {EXCHANGE_CME, longfist::enums::InstrumentType::Future},
    {EXCHANGE_IMM, longfist::enums::InstrumentType::Future},
    {EXCHANGE_WIDX, longfist::enums::InstrumentType::Future},
    {EXCHANGE_FREX, longfist::enums::InstrumentType::Future},
    {EXCHANGE_METL, longfist::enums::InstrumentType::Future},
    {EXCHANGE_IPM, longfist::enums::InstrumentType::Future}};

class wingchun_error : public std::runtime_error {
public:
  explicit wingchun_error(const std::string &__s) : std::runtime_error(__s) {}

  explicit wingchun_error(const char *__s) : std::runtime_error(__s) {}

  virtual ~wingchun_error() noexcept = default;
};

inline bool is_greater(double x, double y) { return std::isgreater(x - y, EPSILON); }

inline bool is_less(double x, double y) { return std::isless(x - y, EPSILON * -1); }

inline bool is_equal(double x, double y) { return std::abs(x - y) <= EPSILON * std::abs(x); }

inline bool is_greater_equal(double x, double y) { return is_greater(x, y) || is_equal(x, y); }

inline bool is_less_equal(double x, double y) { return is_less(x, y) || is_equal(x, y); }

inline bool is_zero(double x) { return is_equal(x, 0.0); }

inline bool is_too_large(double x) { return is_greater(x, DOUBLEMAX); }

inline bool is_valid_price(double price) { return !is_less_equal(price, 0.0) && !is_too_large(price); }

inline double rounded(double x, int n) {
  if (is_too_large(x) || is_zero(x) || is_too_large(std::abs(x))) {
    return 0.0;
  } else {
    char out[64];
    double xrounded;
    sprintf(out, "%.*f", n, x);
    xrounded = strtod(out, 0);
    return xrounded;
  }
}

inline bool string_equals(const std::string &s1, const std::string &s2) {
  return std::strcmp(s1.c_str(), s2.c_str()) == 0;
}

inline bool string_equals_n(const std::string &s1, const std::string &s2, size_t l) {
  return std::strncmp(s1.c_str(), s2.c_str(), l) == 0;
}

inline bool endswith(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

inline bool startswith(const std::string &str, const std::string &prefix) {
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

inline bool is_final_status(const longfist::enums::OrderStatus &status) {
  switch (status) {
  case longfist::enums::OrderStatus::Submitted:
  case longfist::enums::OrderStatus::Pending:
  case longfist::enums::OrderStatus::PartialFilledActive:
  case longfist::enums::OrderStatus::Unknown:
    return false;
  default:
    return true;
  }
}

inline bool is_convertible_bond(const std::string &instrument_id, const std::string &exchange_id) {
  return ((string_equals_n(instrument_id, "123", 3) || string_equals_n(instrument_id, "128", 3) ||
           string_equals_n(instrument_id, "117", 3) || string_equals_n(instrument_id, "127", 3) ||
           string_equals_n(instrument_id, "125", 3) || string_equals_n(instrument_id, "126", 3)) &&
          string_equals(exchange_id, EXCHANGE_SZE)) ||
         ((string_equals_n(instrument_id, "110", 3) || string_equals_n(instrument_id, "113", 3)) &&
          string_equals(exchange_id, EXCHANGE_SSE));
}

inline bool is_repo(const std::string &instrument_id, const std::string &exchange_id) {
  return (string_equals_n(instrument_id, "204", 3) && string_equals(exchange_id, EXCHANGE_SSE)) ||
         (string_equals_n(instrument_id, "1318", 4) && string_equals(exchange_id, EXCHANGE_SZE));
}

inline double get_repo_commission_rate(const std::string &instrument_id) {
  if (string_equals(instrument_id, "204001") || string_equals(instrument_id, "131810")) {
    return 0.00001;
  } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811")) {
    return 0.00002;
  } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800")) {
    return 0.00003;
  } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809")) {
    return 0.00004;
  } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801")) {
    return 0.00005;
  } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802")) {
    return 0.00010;
  } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803")) {
    return 0.00020;
  } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805")) {
    return 0.00030;
  } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806")) {
    return 0.00030;
  } else {
    return 0.0;
  }
}

inline int get_repo_expire_days(const std::string &instrument_id) {
  if (string_equals(instrument_id, "204001") || string_equals(instrument_id, "131810")) {
    return 1;
  } else if (string_equals(instrument_id, "204002") || string_equals(instrument_id, "131811")) {
    return 2;
  } else if (string_equals(instrument_id, "204003") || string_equals(instrument_id, "131800")) {
    return 3;
  } else if (string_equals(instrument_id, "204004") || string_equals(instrument_id, "131809")) {
    return 4;
  } else if (string_equals(instrument_id, "204007") || string_equals(instrument_id, "131801")) {
    return 7;
  } else if (string_equals(instrument_id, "204014") || string_equals(instrument_id, "131802")) {
    return 14;
  } else if (string_equals(instrument_id, "204029") || string_equals(instrument_id, "131803")) {
    return 28;
  } else if (string_equals(instrument_id, "204091") || string_equals(instrument_id, "131805")) {
    return 91;
  } else if (string_equals(instrument_id, "204182") || string_equals(instrument_id, "131806")) {
    return 182;
  } else {
    return 0;
  }
}

inline longfist::enums::InstrumentType get_instrument_type(const std::string &exchange_id,
                                                           const std::string &instrument_id) {
  auto iter = g_exchange_id_type_map_.find(exchange_id);

  if (iter != g_exchange_id_type_map_.end()) {
    return iter->second;
  }

  if (string_equals(exchange_id, EXCHANGE_SSE)) {
    if (startswith(instrument_id, "00")) {
      return longfist::enums::InstrumentType::Index;
    } else if (startswith(instrument_id, "0")) {
      return longfist::enums::InstrumentType::Bond;
    } else if (startswith(instrument_id, "1")) {
      return longfist::enums::InstrumentType::Bond;
    } else if (startswith(instrument_id, "2")) {
      return longfist::enums::InstrumentType::Repo;
    } else if (startswith(instrument_id, "5")) {
      return longfist::enums::InstrumentType::Fund;
    }
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_SZE)) {
    if (startswith(instrument_id, "15") || startswith(instrument_id, "16") || startswith(instrument_id, "18")) {
      return longfist::enums::InstrumentType::Fund;
    } else if (startswith(instrument_id, "13")) {
      return longfist::enums::InstrumentType::Repo;
    } else if (startswith(instrument_id, "1")) {
      return longfist::enums::InstrumentType::Bond;
    }
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_BSE)) {
    return longfist::enums::InstrumentType::Stock;
  } else if (string_equals(exchange_id, EXCHANGE_DCE) || string_equals(exchange_id, EXCHANGE_SHFE) ||
             string_equals(exchange_id, EXCHANGE_CFFEX) || string_equals(exchange_id, EXCHANGE_CZCE) ||
             string_equals(exchange_id, EXCHANGE_INE)) {
    return longfist::enums::InstrumentType::Future;
  } else if (string_equals(exchange_id, EXCHANGE_BINANCE) || string_equals(exchange_id, EXCHANGE_HB)) {
    return longfist::enums::InstrumentType::Crypto;
  }
  SPDLOG_ERROR("invalid instrument type for exchange {} and instrument {}", exchange_id, instrument_id);
  return longfist::enums::InstrumentType::Unknown;
}

inline std::string str_from_instrument_type(longfist::enums::InstrumentType type) {
  switch (type) {
  case longfist::enums::InstrumentType::Unknown:
    return "Unknown";
  case longfist::enums::InstrumentType::Stock:
    return "Stock";
  case longfist::enums::InstrumentType::Future:
    return "Future";
  case longfist::enums::InstrumentType::Bond:
    return "Bond";
  case longfist::enums::InstrumentType::StockOption:
    return "StockOption";
  case longfist::enums::InstrumentType::Fund:
    return "Fund";
  case longfist::enums::InstrumentType::TechStock:
    return "TechStock";
  case longfist::enums::InstrumentType::Index:
    return "Index";
  case longfist::enums::InstrumentType::Repo:
    return "Repo";
  case longfist::enums::InstrumentType::Crypto:
    return "Crypto";
  default:
    return "Unknown";
  }
}

inline std::string get_instrument_product(const char *instrument_id) {
  std::string product = {};
  int i = 0;
  while (instrument_id[i] != 0) {
    if (instrument_id[i] < '0' || instrument_id[i] > '9') {
      product.push_back(std::toupper(instrument_id[i]));
    }
    ++i;
  }
  return product;
}

inline longfist::enums::Direction get_future_direction(longfist::enums::Side side, longfist::enums::Offset offset) {
  if ((side == longfist::enums::Side::Buy && offset == longfist::enums::Offset::Open) ||
      (side == longfist::enums::Side::Sell && offset != longfist::enums::Offset::Open)) {
    return longfist::enums::Direction::Long;
  } else {
    return longfist::enums::Direction::Short;
  }
}

inline bool is_open(longfist::enums::InstrumentType instrument_type, longfist::enums::Side side,
                    longfist::enums::Offset offset) {
  if (instrument_type == longfist::enums::InstrumentType::Stock ||
      instrument_type == longfist::enums::InstrumentType::Bond) {
    return side == longfist::enums::Side::Buy;
  } else if (instrument_type == longfist::enums::InstrumentType::Future) {
    return offset == longfist::enums::Offset::Open;
  } else {
    return false;
  }
}

inline std::string get_exchange_id_from_future_instrument_id(const std::string &instrument_id) {
  std::size_t found = instrument_id.find_first_of("0123456789");
  std::string product = instrument_id.substr(0, found);
  std::transform(product.begin(), product.end(), product.begin(), ::tolower);
  if (product == "c" || product == "cs" || product == "a" || product == "b" || product == "m" || product == "y" ||
      product == "p" || product == "fb" || product == "bb" || product == "jd" || product == "rr" || product == "l" ||
      product == "v" || product == "pp" || product == "j" || product == "jm" || product == "i" || product == "eg" ||
      product == "eb") {
    return EXCHANGE_DCE;
  } else if (product == "wh" || product == "pm" || product == "cf" || product == "sr" || product == "oi" ||
             product == "ri" || product == "rs" || product == "rm" || product == "jr" || product == "lr" ||
             product == "cy" || product == "ap" || product == "cj" || product == "ta" || product == "ma" ||
             product == "zc" || product == "sf" || product == "sm" || product == "ur" || product == "fg" ||
             product == "sa") {
    return EXCHANGE_CZCE;
  } else if (product == "cu" || product == "al" || product == "zn" || product == "pb" || product == "ni" ||
             product == "sn" || product == "au" || product == "ag" || product == "rb" || product == "wr" ||
             product == "hc" || product == "ss" || product == "fu" || product == "bu" || product == "ru" ||
             product == "nr" || product == "sp") {
    return EXCHANGE_SHFE;
  } else if (product == "if" || product == "ic" || product == "ih" || product == "ts" || product == "tf" ||
             product == "t") {
    return EXCHANGE_CFFEX;
  } else if (product == "sc") {
    return EXCHANGE_INE;
  } else {
    return "";
  }
}

inline bool is_shortable(longfist::enums::InstrumentType instrument_type) {
  using namespace longfist::enums;
  return not(instrument_type == InstrumentType::Stock or instrument_type == InstrumentType::Bond or
             instrument_type == InstrumentType::Fund or instrument_type == InstrumentType::StockOption or
             instrument_type == InstrumentType::TechStock or instrument_type == InstrumentType::Index or
             instrument_type == InstrumentType::Repo);
}

inline longfist::enums::Direction get_direction(longfist::enums::InstrumentType instrument_type,
                                                longfist::enums::Side side, longfist::enums::Offset offset) {
  using namespace longfist::enums;

  if (side == Side::MarginTrade) {
    return Direction::Long;
  } else if (side == Side::ShortSell) {
    return Direction::Short;
  } else if (side == Side::RepayMargin) {
    return Direction::Long;
  } else if (side == Side::RepayStock) {
    return Direction::Short;
  }
  if (not is_shortable(instrument_type)) {
    return Direction::Long;
  }
  if (side == Side::Buy and offset == Offset::Open) {
    return Direction::Long;
  }
  if (side == Side::Sell and
      (offset == Offset::Close or offset == Offset::CloseToday or offset == Offset::CloseYesterday)) {
    return Direction::Long;
  }
  if (side == Side::Sell and offset == Offset::Open) {
    return Direction::Short;
  }
  if (side == Side::Buy and
      (offset == Offset::Close or offset == Offset::CloseToday or offset == Offset::CloseYesterday)) {
    return Direction::Short;
  }
  throw wingchun_error(fmt::format("get_direction error: invalid direction args {} {} {}", (int)instrument_type, (int)side, (int)offset));
}

inline uint32_t hash_instrument(const char *exchange_id, const char *instrument_id) {
  return yijinjing::util::hash_str_32(instrument_id) ^ yijinjing::util::hash_str_32(exchange_id);
}

inline void order_from_input(const longfist::types::OrderInput &input, longfist::types::Order &order) {
  order.parent_id = input.parent_id;
  order.order_id = input.order_id;

  strcpy(order.instrument_id, input.instrument_id);
  strcpy(order.exchange_id, input.exchange_id);
  strcpy(order.source_id, input.source_id);
  strcpy(order.account_id, input.account_id);

  order.instrument_type = input.instrument_type;

  order.limit_price = input.limit_price;
  order.frozen_price = input.frozen_price;

  order.volume = input.volume;
  order.volume_traded = 0;
  order.volume_left = input.volume;
  order.status = longfist::enums::OrderStatus::Submitted;

  order.side = input.side;
  order.offset = input.offset;
  order.hedge_flag = input.hedge_flag;

  order.price_type = input.price_type;
  order.volume_condition = input.volume_condition;
  order.time_condition = input.time_condition;
}


/*****************************************************************************
*  @Copyright (c) 2022, Marsjliu
*  @All rights reserved

*  @date     : 2022/06/06 17:15
*  @brief    :根据exchangeid和时间戳转换成本地YYYYMMDD
*****************************************************************************/
// ExchangeId与对应的LocationTime类型
static const std::unordered_map<std::string, kungfu::yijinjing::LocationTimeType> g_mapLocationTimeExchangeId = {
    {EXCHANGE_US, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_HK, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_SSE, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_SZE, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_BSE, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_SHFE, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_HK_OPTION, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_GLFX, kungfu::yijinjing::LocationTimeType::Beijing},
    {EXCHANGE_IPE, kungfu::yijinjing::LocationTimeType::London},
    {EXCHANGE_CBOT, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_CEC, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_LIFE, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_MTIF, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_NYCE, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_CMX, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_SIME, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_CME, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_IMM, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_WIDX, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_FREX, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_METL, kungfu::yijinjing::LocationTimeType::AmericaEastern},
    {EXCHANGE_IPM, kungfu::yijinjing::LocationTimeType::London}};

// 根据标准时间戳转换成交易所所在区域的时间
// param1:时间戳(精确到秒)
// param2:交易所id，exchangeid
// params3: 返回时间格式YYYYMMDD
inline std::string TranslateGMTimeToLocalDateByExchangeId(time_t lTime, const std::string &exchangeId, const std::string strformat = "%Y%m%d") {
  kungfu::yijinjing::LocationTimeType type = kungfu::yijinjing::LocationTimeType::Beijing;
  std::unordered_map<std::string, kungfu::yijinjing::LocationTimeType>::const_iterator it = g_mapLocationTimeExchangeId.find(exchangeId);

  if (it != g_mapLocationTimeExchangeId.end()) {
    type = it->second;
  }

  __int64 local_time = kungfu::yijinjing::TimeUtil::TranslateGMTimeToLocalTime(lTime, type)->seconds;

  char datebuf[256] = {0};
  strftime(datebuf, 256, strformat.c_str(), gmtime(&local_time));
  return datebuf;
}

} // namespace kungfu::wingchun

#endif // WINGCHUN_COMMON_H
