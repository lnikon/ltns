#pragma once

#include "NetworkMonitor/Stomp/StompParser.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>

namespace Networking::Stomp {

using TokenValue = std::variant<std::monostate, std::string, std::size_t>;

template <class EnumType> class EnumToStringBimap {
  using bimap_type = boost::bimaps::
    bimap<boost::bimaps::set_of<EnumType>, boost::bimaps::set_of<std::string>>;

public:
  EnumToStringBimap() = delete;

  EnumToStringBimap(
    const std::vector<std::pair<EnumType, std::string>> &mappings)
  {
    std::ranges::for_each(mappings, [this](auto mapping) {
      m_tokenTypeToString.insert(
        {std::move(mapping.first), std::move(mapping.second)});
    });
  }

  EnumToStringBimap(const EnumToStringBimap<EnumType> &) = default;
  auto operator=(const EnumToStringBimap<EnumType> &)
    -> EnumToStringBimap<EnumType> & = default;

  EnumToStringBimap(EnumToStringBimap<EnumType> &&) noexcept = default;
  auto operator=(EnumToStringBimap<EnumType> &&) noexcept
    -> EnumToStringBimap<EnumType> & = default;

  ~EnumToStringBimap() = default;

  [[nodiscard]] auto ToString(const EnumType type) const
    -> std::optional<std::string>
  {
    const auto cit{m_tokenTypeToString.left.find(type)};
    return cit != m_tokenTypeToString.left.end()
             ? std::make_optional(cit->second)
             : std::nullopt;
  }

  [[nodiscard]] auto ToEnum(const std::string &type) const
    -> std::optional<EnumType>
  {
    const auto cit{m_tokenTypeToString.right.find(type)};
    return cit != m_tokenTypeToString.right.end()
             ? std::make_optional(cit->second)
             : std::nullopt;
  }

private:
  bimap_type m_tokenTypeToString;
};

enum class StompCommand {
  kUndefined = 0,
  kSTOMP = 1,

  kSizeOfEnum
};

class StompCommmandToStringBimap : public EnumToStringBimap<StompCommand> {
  using Base = EnumToStringBimap<StompCommand>;

public:
  StompCommmandToStringBimap()
      : Base(
          {{StompCommand::kUndefined, "Undefined"},
           {StompCommand::kSTOMP, "STOMP"}})
  {
  }
};

using HeaderKey = std::string;
using HeaderValue = TokenValue;

struct StompHeader {
  HeaderKey m_header;
	HeaderValue m_value;
};

struct StompBody {
  std::string m_body{};
};

struct StompFrame {
  StompCommand m_command{StompCommand::kSTOMP};
  std::vector<StompHeader> m_headers{};
  StompBody m_body;
};

} // namespace Networking::Stomp
