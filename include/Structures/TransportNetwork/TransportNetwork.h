#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/indexed_by.hpp>
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

  bool operator==(const Route &other) const;
  bool operator!=(const Route &other) const;
};

class Station {
public:
  Station(StationId id, StationName name, std::size_t passengerCount = 0);

  Station() = default;

  Station(const Station &) = default;
  Station &operator=(const Station &) = default;

  Station(Station &&) = default;
  Station &operator=(Station &&) = default;

  ~Station() = default;

  bool RecordPassengerEvent(const PassengerEvent &event);
  std::size_t GetPassengerCount() const;

  bool operator==(const Station &rhs) const noexcept {
    return m_id == rhs.m_id && m_name == rhs.m_name;
  }

	bool AddRoute(std::shared_ptr<Route> pRoute);
	std::vector<std::shared_ptr<Route>> GetRoutes() const;

public:
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

  bool operator==(const Line &line) const;
  bool operator!=(const Line &line) const;
};

struct TravelTime {
  StationId m_startStationId{};
  StationId m_endStationId{};
  LineId m_lineId{};
  RouteId m_routeId{};
  double m_travelTime{};
};

using TravelTimes = boost::multi_index_container<
  TravelTime,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::composite_key<
      boost::multi_index::
        member<TravelTime, StationId, &TravelTime::m_startStationId>,
      boost::multi_index::
        member<TravelTime, StationId, &TravelTime::m_endStationId>>>>>;

class TransportNetwork {
public:
  TransportNetwork() = default;

  TransportNetwork(const TransportNetwork &) = default;
  TransportNetwork(TransportNetwork &&) = default;

  TransportNetwork &operator=(const TransportNetwork &) = default;
  TransportNetwork &operator=(TransportNetwork &&) = default;

  ~TransportNetwork() = default;

  bool AddStation(Station station);
  std::shared_ptr<Station> GetStation(StationId id) const;

  bool AddLine(Line line);
  std::shared_ptr<Line> GetLine(LineId lineId) const;

  bool RecordPassengerEvent(const PassengerEvent &event);
  std::size_t GetPassengerCount(const StationId& stationId) const;

  std::vector<std::shared_ptr<Route>> GetRoutesServingStation(const StationId& id) const;

private:
  std::unordered_map<LineId, std::shared_ptr<Line>> m_lines{};
  std::unordered_map<StationId, std::shared_ptr<Station>> m_stations{};
  std::unordered_map<StationId, std::size_t> m_passengerEvents{};
  TravelTimes m_travelTimes;
};

} // namespace Structures::TransportNetwork
