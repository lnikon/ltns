#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ordered_index_fwd.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index_container_fwd.hpp>

namespace Structures::TransportNetwork {

using LineId = std::string;
using RouteId = std::string;
using StationId = std::string;
using StationName = std::string;

enum class RouteDirection {
  kInbound = 0,
  kOutbound,

  kSizeOfEnum
};

struct PassengerEvent {
  enum class Type {
    kIn = 0,
    kOut,

    kSizeOfEnum
  };

  StationId m_stationId{};
  Type m_type{};
};

struct Route {
  LineId lineId{};
  RouteId routeId{};
  RouteDirection direction{};
  StationId startStationId{};
  StationId endStationId{};
  std::vector<StationId> stops{};

  auto operator==(const Route &other) const -> bool;
  auto operator!=(const Route &other) const -> bool;
};

class Station {
public:
  Station(StationId stationId, StationName name, std::size_t passengerCount = 0);

  Station() = default;

  Station(const Station &) = default;
  auto operator=(const Station &) -> Station & = default;

  Station(Station &&) = default;
  auto operator=(Station &&) -> Station & = default;

  ~Station() = default;

  auto RecordPassengerEvent(const PassengerEvent &event) -> bool;
  [[nodiscard]] auto GetPassengerCount() const -> std::size_t;

  auto operator==(const Station &rhs) const noexcept -> bool;

  auto AddRoute(std::shared_ptr<Route> pRoute) -> bool;
  [[nodiscard]] auto GetRoutes() const -> std::vector<std::shared_ptr<Route>>;

  StationId m_id{};
  StationName m_name{};
  std::vector<std::shared_ptr<Route>> m_routes{};

private:
  std::size_t m_passengerCount{0};
};

struct Line {
  LineId id{};
  std::string name{};
  std::vector<std::shared_ptr<Route>> routes{};

  auto operator==(const Line &line) const -> bool;
  auto operator!=(const Line &line) const -> bool;
};

struct TravelTime {
  StationId m_startStationId{};
  StationId m_endStationId{};
  LineId m_lineId{};
  RouteId m_routeId{};
  unsigned int m_travelTime{};
};

using TravelTimes = boost::multi_index_container<
  TravelTime,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::composite_key<
      TravelTime,
      boost::multi_index::
        member<TravelTime, StationId, &TravelTime::m_startStationId>,
      boost::multi_index::
        member<TravelTime, StationId, &TravelTime::m_endStationId>>>>>;

class TransportNetwork {
public:
  TransportNetwork() = default;

  TransportNetwork(const TransportNetwork &) = default;
  TransportNetwork(TransportNetwork &&) = default;

  auto operator=(const TransportNetwork &) -> TransportNetwork & = default;
  auto operator=(TransportNetwork &&) -> TransportNetwork & = default;

  ~TransportNetwork() = default;

  auto AddStation(Station station) -> bool;
  auto GetStation(const StationId& stationId) const -> std::shared_ptr<Station>;

  auto AddLine(Line line) -> bool;
  auto GetLine(const LineId& lineId) const -> std::shared_ptr<Line>;

  auto RecordPassengerEvent(const PassengerEvent &event) const -> bool;
  auto GetPassengerCount(const StationId &stationId) const -> std::size_t;

  auto
  GetRoutesServingStation(const StationId &stationId) const -> std::vector<std::shared_ptr<Route>>;

  auto SetTravelTime(
    const StationId &start,
    const StationId &end,
    unsigned int travelTime) -> bool;

  auto
  GetTravelTime(const StationId &start, const StationId &end) const -> unsigned int;

private:
  std::unordered_map<LineId, std::shared_ptr<Line>> m_lines{};
  std::unordered_map<StationId, std::shared_ptr<Station>> m_stations{};
  std::unordered_map<StationId, std::size_t> m_passengerEvents{};
  TravelTimes m_travelTimes{};
};

} // namespace Structures::TransportNetwork
