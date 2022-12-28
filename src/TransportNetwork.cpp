#include <TransportNetwork/TransportNetwork.h>

#include <bits/ranges_algobase.h>
#include <memory>
#include <numbers>
#include <ranges>
#include <stdexcept>

namespace Structures::TransportNetwork {

bool Route::operator==(const Route &other) const
{
  return lineId == other.lineId && routeId == other.routeId &&
         direction == other.direction &&
         startStationId == other.startStationId &&
         endStationId == other.endStationId && stops == other.stops;
}

bool Route::operator!=(const Route &other) const { return !(*this == other); }

Station::Station(StationId id, StationName name, std::size_t passengerCount)
    : m_id(std::move(id)),
      m_name(std::move(name)),
      m_passengerCount(passengerCount)
{
}

bool Station::RecordPassengerEvent(const PassengerEvent &event)
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

std::size_t Station::GetPassengerCount() const { return m_passengerCount; }

bool Line::operator==(const Line &line) const
{
  const bool ok = id == line.id && name == line.name && routes == line.routes;
  return ok;
}

bool Line::operator!=(const Line &line) const { return !(*this == line); }

bool TransportNetwork::AddStation(Station station)
{
  assert(!station.m_id.empty());
  assert(!station.m_name.empty());

  auto id{station.m_id};
  auto res{m_stations.emplace(
    std::move(id),
    std::make_shared<Station>(std::move(station)))};

  return res.second;
}

std::shared_ptr<Station> TransportNetwork::GetStation(StationId id) const
{
  assert(!id.empty());

  auto it = m_stations.find(id);
  return (it != m_stations.end() ? it->second : nullptr);
}

bool TransportNetwork::AddLine(Line line)
{
  // Lines with empty routes are not supported
  if (line.routes.empty()) {
    throw std::logic_error(
      "(TransportNetwork::AddLine): Line with empty routes are not supported!");
  }

  // When inserting the line, network should already contain all its stations.
  for (const auto &route : line.routes) {
    for (const auto &stationId : route.stops) {
      if (!GetStation(stationId)) {
        throw std::logic_error(
          "(TransportNetwork::AddLine): Network contains no station=" +
          stationId);
      }
    }
  }

  // Check that routes are in correct order.
  for (std::size_t current{0}, next{1}; next < line.routes.size();
       current = next, ++next) {
    if (line.routes[current].endStationId != line.routes[next].startStationId) {
      throw std::logic_error(
        "(TransportNetwork::AddLine): Network line contains a gap!");
    }
  }

  auto lineId{line.id};
  auto res{m_lines.emplace(
    std::move(lineId),
    std::make_shared<Line>(std::move(line)))};

  return res.second;
}

std::shared_ptr<Line> TransportNetwork::GetLine(LineId lineId) const
{
  assert(!lineId.empty());

  auto it = m_lines.find(lineId);
  return (it != m_lines.end() ? it->second : nullptr);
}

bool TransportNetwork::RecordPassengerEvent(const PassengerEvent &event)
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

std::size_t
TransportNetwork::GetPassengerCount(const StationId &stationId) const
{
  assert(!stationId.empty());

  auto pStation{GetStation(stationId)};
  if (pStation) {
    return pStation->GetPassengerCount();
  }

  return 0;
}

} // namespace Structures::TransportNetwork
