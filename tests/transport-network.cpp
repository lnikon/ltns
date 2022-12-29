#include <Structures/TransportNetwork/TransportNetwork.h>
#include <Structures/TransportNetwork/TransportNetworkParser.h>

#include <boost/mpl/begin_end.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <stdexcept>

using namespace Structures::TransportNetwork;

BOOST_AUTO_TEST_SUITE(TransportNetworkTestSuite);

BOOST_AUTO_TEST_CASE(AddStationAndGetIt)
{
  TransportNetwork tn{};
  const Station st1("station_001", "Bagramyan");

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(st1 == *tn.GetStation(st1.m_id));
}

BOOST_AUTO_TEST_CASE(AddDuplicateStation)
{
  TransportNetwork tn{};

  // Both stations have the same id, but differ in the name.
  const Station st1("station_001", "Bagramyan");
  const Station st2("station_001", "Yeritasardakan");

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(!tn.AddStation(st2));

  BOOST_CHECK(st1 == *tn.GetStation(st1.m_id));
  BOOST_CHECK(st2 != *tn.GetStation(st1.m_id));
}

BOOST_AUTO_TEST_CASE(AddLineHappyPath)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(tn.AddStation(st2));

  BOOST_CHECK(st1 == *tn.GetStation(st1.m_id));
  BOOST_CHECK(st2 == *tn.GetStation(st2.m_id));

  BOOST_CHECK_NO_THROW(tn.AddLine(ln1));
  BOOST_CHECK(tn.GetLine(lineId) != nullptr);
  BOOST_CHECK(ln1 == *tn.GetLine(lineId));

	BOOST_CHECK(!tn.GetRoutesServingStation(st1.m_id).empty());
	BOOST_CHECK(*tn.GetRoutesServingStation(st1.m_id)[0] == rt1);

	BOOST_CHECK(!tn.GetRoutesServingStation(st2.m_id).empty());
	BOOST_CHECK(*tn.GetRoutesServingStation(st2.m_id)[0] == rt1);
}

BOOST_AUTO_TEST_CASE(AddLineWithMissingStations)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};

  BOOST_CHECK(tn.AddStation(st1));

  // BOOST_CHECK(st1 == *tn.GetStation(st1.m_id));
  BOOST_CHECK(tn.GetStation(st2.m_id) == nullptr);
  BOOST_CHECK_THROW(tn.AddLine(ln1), std::logic_error);
  BOOST_CHECK(tn.GetLine(lineId) == nullptr);
}

BOOST_AUTO_TEST_CASE(AddLineWithMultipleRoutes)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId immStationId1{"station_002"};
  const StationId immStationId2{"station_003"};
  const StationId immStationId3{"station_004"};
  const StationId endStationId{"station_005"};
  const Station startSt(startStationId, "Bagramyan");
  const Station immSt1(immStationId1, "Yeritasardakan");
  const Station immSt2(immStationId2, "SasuntsiDavid");
  const Station immSt3(immStationId3, "Gorcaranayin");
  const Station endSt(endStationId, "ZoravarAndranik");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{immStationId1},
    .stops{startStationId, immStationId1}};
  const Route rt2{
    .lineId{lineId},
    .routeId{"route_002"},
    .direction = RouteDirection::kInbound,
    .startStationId{immStationId1},
    .endStationId{immStationId2},
    .stops{immStationId1, immStationId2}};
  const Route rt3{
    .lineId{lineId},
    .routeId{"route_003"},
    .direction = RouteDirection::kInbound,
    .startStationId{immStationId2},
    .endStationId{immStationId3},
    .stops{immStationId2, immStationId3}};
  const Route rt4{
    .lineId{lineId},
    .routeId{"route_004"},
    .direction = RouteDirection::kInbound,
    .startStationId{immStationId3},
    .endStationId{endStationId},
    .stops{immStationId3, endStationId}};

  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{
      std::make_shared<Route>(rt1),
      std::make_shared<Route>(rt2),
      std::make_shared<Route>(rt3),
      std::make_shared<Route>(rt4)}};

  BOOST_CHECK(tn.AddStation(startSt));
  BOOST_CHECK(tn.AddStation(immSt1));
  BOOST_CHECK(tn.AddStation(immSt2));
  BOOST_CHECK(tn.AddStation(immSt3));
  BOOST_CHECK(tn.AddStation(endSt));
  BOOST_CHECK(tn.AddLine(ln1));

  BOOST_CHECK(ln1 == *tn.GetLine(lineId));
}

BOOST_AUTO_TEST_CASE(AddLineWithGap)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId immStationId1{"station_002"};
  const StationId immStationId2{"station_003"};
  const StationId immStationId3{"station_004"};
  const StationId endStationId{"station_005"};
  const Station startSt(startStationId, "Bagramyan");
  const Station immSt1(immStationId1, "Yeritasardakan");
  const Station immSt2(immStationId2, "SasuntsiDavid");
  const Station immSt3(immStationId3, "Gorcaranayin");
  const Station endSt(endStationId, "ZoravarAndranik");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{immStationId1},
    .stops{startStationId, immStationId1}};
  const Route rt2{
    .lineId{lineId},
    .routeId{"route_002"},
    .direction = RouteDirection::kInbound,
    .startStationId{immStationId1},
    .endStationId{immStationId2},
    .stops{immStationId1, immStationId2}};
  // Instead of route three we have a gap!
  // const Route rt3{
  //  .lineId{lineId},
  //  .routeId{"route_003"},
  //  .direction = RouteDirection::kInbound,
  //  .startStationId{immStationId2},
  //  .endStationId{immStationId3},
  //  .stops{immStationId2, immStationId3}};
  const Route rt4{
    .lineId{lineId},
    .routeId{"route_004"},
    .direction = RouteDirection::kInbound,
    .startStationId{immStationId3},
    .endStationId{endStationId},
    .stops{immStationId3, endStationId}};

  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{
      std::make_shared<Route>(rt1),
      std::make_shared<Route>(rt2),
      std::make_shared<Route>(rt4)}};

  BOOST_CHECK(tn.AddStation(startSt));
  BOOST_CHECK(tn.AddStation(immSt1));
  BOOST_CHECK(tn.AddStation(immSt2));
  BOOST_CHECK(tn.AddStation(immSt3));
  BOOST_CHECK(tn.AddStation(endSt));

  BOOST_CHECK_NO_THROW(tn.AddLine(ln1));
  BOOST_CHECK(tn.GetLine(lineId) != nullptr);
  BOOST_CHECK(*tn.GetLine(lineId) == ln1);
}

BOOST_AUTO_TEST_CASE(AddLineWithEmptyRoutes)
{
  const LineId lineId{"empty_line"};
  TransportNetwork tn{};
  Line ln{.id{lineId}, .name{"empty_name"}, .routes{}};

  BOOST_CHECK_THROW(tn.AddLine(ln), std::logic_error);
  BOOST_CHECK(tn.GetLine(lineId) == nullptr);
}

BOOST_AUTO_TEST_CASE(RecordPassengerEventForExistingStation)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};
  PassengerEvent event{
    .m_stationId{startStationId},
    .m_type = PassengerEvent::Type::kIn};

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(tn.AddStation(st2));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(startStationId), 3);
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 0);

  event.m_type = PassengerEvent::Type::kOut;

  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(startStationId), 1);

  event.m_stationId = endStationId;
  event.m_type = PassengerEvent::Type::kIn;

  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 0);
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 3);

  event.m_type = PassengerEvent::Type::kOut;

  BOOST_CHECK(tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 2);
}

BOOST_AUTO_TEST_CASE(RecordOutPassengerEventForEmptyStation)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};
  PassengerEvent event{
    .m_stationId{startStationId},
    .m_type = PassengerEvent::Type::kOut};

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(tn.AddStation(st2));
  BOOST_CHECK(!tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(startStationId), 0);
}

BOOST_AUTO_TEST_CASE(RecordPassengerEventForNonExistingStation)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};
  const PassengerEvent event{
    .m_stationId{startStationId},
    .m_type = PassengerEvent::Type::kIn};

  // Add station2 instead of 1.
  BOOST_CHECK(tn.AddStation(st2));
  BOOST_CHECK(!tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(startStationId), 0);
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 0);
  BOOST_CHECK(tn.GetRoutesServingStation(startStationId).empty());
  BOOST_CHECK(tn.GetRoutesServingStation(endStationId).empty());
}

BOOST_AUTO_TEST_CASE(RecordPassengerEventWithWrongType)
{
  TransportNetwork tn{};

  const LineId lineId{"line_001"};
  const StationId startStationId{"station_001"};
  const StationId endStationId{"station_002"};
  const Station st1(startStationId, "Bagramyan");
  const Station st2(endStationId, "Yeritasardakan");
  const Route rt1{
    .lineId{lineId},
    .routeId{"route_001"},
    .direction = RouteDirection::kInbound,
    .startStationId{startStationId},
    .endStationId{endStationId},
    .stops{startStationId, endStationId}};
  const Line ln1{
    .id{lineId},
    .name{"bagyer"},
    .routes{std::make_shared<Route>(rt1)}};
  const PassengerEvent event{
    .m_stationId{startStationId},
    .m_type = PassengerEvent::Type::kSizeOfEnum};

  BOOST_CHECK(tn.AddStation(st1));
  BOOST_CHECK(!tn.RecordPassengerEvent(event));
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(startStationId), 0);
  BOOST_CHECK_EQUAL(tn.GetPassengerCount(endStationId), 0);
}

BOOST_AUTO_TEST_SUITE_END()
