#include <TransportNetwork/TransportNetwork.h>

#include <bits/ranges_algobase.h>
#include <bits/ranges_util.h>
#include <memory>
#include <numbers>
#include <ranges>
#include <stdexcept>

namespace Structures::TransportNetwork {

auto Route::operator==(const Route &other) const -> bool
{
  return lineId == other.lineId && routeId == other.routeId &&
         direction == other.direction &&
         startStationId == other.startStationId &&
         endStationId == other.endStationId && stops == other.stops;
}

auto Route::operator!=(const Route &other) const -> bool
{
  return !(*this == other);
}

Station::Station(StationId id, StationName name, std::size_t passengerCount)
    : m_id(std::move(id)),
      m_name(std::move(name)),
      m_passengerCount(passengerCount)
{
}

auto Station::RecordPassengerEvent(const PassengerEvent &event) -> bool
{
  switch (event.m_type) {
    case PassengerEvent::Type::kIn:
      m_passengerCount++;
      break;
    case PassengerEvent::Type::kOut:
      if (m_passengerCount == 0) {
        return false;
      }

      m_passengerCount--;
      break;
    default:
      return false;
  }

  return true;
}

auto Station::GetPassengerCount() const -> std::size_t
{
  return m_passengerCount;
}

auto Station::AddRoute(std::shared_ptr<Route> pRoute) -> bool
{
  assert(pRoute);

  if (const auto cit{std::ranges::find_if(
        m_routes,
        [pRoute](auto existingRoute) { return *pRoute == *existingRoute; })};
      cit != m_routes.end()) {
    return false;
  }

  m_routes.emplace_back(std::move(pRoute));
  return true;
}

auto Station::GetRoutes() const -> std::vector<std::shared_ptr<Route>>
{
  return m_routes;
}

auto Line::operator==(const Line &line) const -> bool
{
  return id == line.id && name == line.name && routes == line.routes;
}

auto Line::operator!=(const Line &line) const -> bool
{
  return !(*this == line);
}

auto TransportNetwork::AddStation(Station station) -> bool
{
  assert(!station.m_id.empty());
  assert(!station.m_name.empty());

  auto stationId{station.m_id};
  auto res{m_stations.emplace(
    std::move(stationId),
    std::make_shared<Station>(std::move(station)))};

  return res.second;
}

auto TransportNetwork::GetStation(const StationId &stationId) const
  -> std::shared_ptr<Station>
{
  assert(!stationId.empty());

  const auto cit = m_stations.find(stationId);
  return (cit != m_stations.end() ? cit->second : nullptr);
}

bool TransportNetwork::AddLine(Line line)
{
  // Lines with empty routes are not supported
  if (line.routes.empty()) {
    throw std::logic_error(
      "(TransportNetwork::AddLine): Line with empty routes are not supported!");
  }

  // TODO: Move into function.
  // When inserting the line, network should already contain all its stations.
  for (const auto &route : line.routes) {
    for (const auto &stationId : route->stops) {
      if (!GetStation(stationId)) {
        throw std::logic_error(
          "(TransportNetwork::AddLine): Network contains no station=" +
          stationId);
      }
    }
  }

  // TODO: Move into function.
  for (const auto &route : line.routes) {
    for (const auto &stationId : route->stops) {
      auto pStation{GetStation(stationId)};
      pStation->AddRoute(route);
    }
  }

  auto lineId{line.id};
  const auto res{m_lines.emplace(
    std::move(lineId),
    std::make_shared<Line>(std::move(line)))};

  return res.second;
}

auto TransportNetwork::GetLine(const LineId &lineId) const
  -> std::shared_ptr<Line>
{
  assert(!lineId.empty());

  const auto cit{m_lines.find(lineId)};
  return (cit != m_lines.end() ? cit->second : nullptr);
}

auto TransportNetwork::RecordPassengerEvent(const PassengerEvent &event) const -> bool
{
  assert(!event.m_stationId.empty());
  // TODO: Maybe throw exception instead of assert?
  // assert(
  //   event.m_type == PassengerEvent::Type::kIn ||
  //   event.m_type == PassengerEvent::Type::kOut);

  auto pStation{GetStation(event.m_stationId)};
  if (pStation) {
    return pStation->RecordPassengerEvent(event);
  }

  return false;
}

auto
TransportNetwork::GetPassengerCount(const StationId &stationId) const -> std::size_t
{
  assert(!stationId.empty());

  auto pStation{GetStation(stationId)};
  if (pStation) {
    return pStation->GetPassengerCount();
  }

  return 0;
}

auto
TransportNetwork::GetRoutesServingStation(const StationId &stationId) const -> std::vector<std::shared_ptr<Route>>
{
  assert(!stationId.empty());

  const auto pStation{GetStation(stationId)};
  if (pStation) {
    return pStation->GetRoutes();
  }

  return {};
}

auto TransportNetwork::SetTravelTime(
  const StationId &start,
  const StationId &end,
  const unsigned int travelTime) -> bool
{
  assert(!start.empty());
  assert(!end.empty());

  if (start == end) {
    return false;
  }

  const auto res{m_travelTimes.insert(TravelTime{
    .m_startStationId{start},
    .m_endStationId{end},
    .m_travelTime = travelTime})};

  return res.second;
}

auto TransportNetwork::GetTravelTime(
  const StationId &start,
  const StationId &end) const -> unsigned int
{
  assert(!start.empty());
  assert(!end.empty());

  const auto res{m_travelTimes.find(boost::make_tuple(start, end))};
  if (res != m_travelTimes.end()) {
    return res->m_travelTime;
  }

  return 0;
}

auto Station::operator==(const Station &rhs) const noexcept -> bool
{
  return m_id == rhs.m_id && m_name == rhs.m_name;
}
} // namespace Structures::TransportNetwork
