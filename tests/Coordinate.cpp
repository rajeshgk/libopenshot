/**
 * @file
 * @brief Unit tests for openshot::Coordinate
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @ref License
 */

// Copyright (c) 2008-2019 OpenShot Studios, LLC
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "openshot_catch.h"

#include "Coordinate.h"
#include "Exceptions.h"

using namespace openshot;

TEST_CASE( "default constructor", "[libopenshot][coordinate]" )
{
	// Create an empty coordinate
	Coordinate c1;

	CHECK(c1.X == Approx(0.0f).margin(0.00001));
	CHECK(c1.Y == Approx(0.0f).margin(0.00001));
}

TEST_CASE( "XY constructor", "[libopenshot][coordinate]" )
{
	// Create an empty coordinate
	Coordinate c1(2,8);

	CHECK(c1.X == Approx(2.0f).margin(0.00001));
	CHECK(c1.Y == Approx(8.0f).margin(0.00001));
}

TEST_CASE( "std::pair constructor", "[libopenshot][coordinate]" )
{
	Coordinate c1(std::pair<double,double>(12, 10));
	CHECK(c1.X == Approx(12.0f).margin(0.00001));
	CHECK(c1.Y == Approx(10.0f).margin(0.00001));
}

TEST_CASE( "Json", "[libopenshot][coordinate]" )
{
	openshot::Coordinate c(100, 200);
	openshot::Coordinate c1;
	c1.X = 100;
	c1.Y = 200;
	// Check that JSON produced is identical
	auto j = c.Json();
	auto j1 = c1.Json();
	CHECK(j1 == j);
	// Check Json::Value representation
	auto jv = c.JsonValue();
	auto jv_string = jv.toStyledString();
	CHECK(j1 == jv_string);
}

TEST_CASE( "SetJson", "[libopenshot][coordinate]" ) {
	// Construct our input Json representation
	const std::string json_input = R"json(
	{
		"X": 100.0,
		"Y": 50.0
	}
		)json";
	openshot::Coordinate c;
	CHECK_THROWS_AS(c.SetJson("}{"), openshot::InvalidJSON);
	// Check that values set via SetJson() are correct
	c.SetJson(json_input);
	CHECK(c.X == Approx(100.0).margin(0.01));
	CHECK(c.Y == Approx(50.0).margin(0.01));
}

// Verify that partial JSON updates preserve existing coordinate values.
TEST_CASE("Coordinate JSON partial updates preserve values",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(10.0, 20.0);

    coordinate.SetJson(R"({"X": 15.5})");
    CHECK(coordinate.X == Approx(15.5));
    CHECK(coordinate.Y == Approx(20.0));

    coordinate.SetJson(R"({"Y": -7.25})");
    CHECK(coordinate.X == Approx(15.5));
    CHECK(coordinate.Y == Approx(-7.25));

    coordinate.SetJson(R"({})");
    CHECK(coordinate.X == Approx(15.5));
    CHECK(coordinate.Y == Approx(-7.25));
}

// Explicit null values must not overwrite existing coordinates.
TEST_CASE("Coordinate JSON null values preserve coordinates",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(12.0, 34.0);

    coordinate.SetJson(R"({"X": null, "Y": null})");

    CHECK(coordinate.X == Approx(12.0));
    CHECK(coordinate.Y == Approx(34.0));
}

// Malformed JSON must raise the established public exception.
TEST_CASE("Coordinate JSON rejects malformed documents",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(1.0, 2.0);

    CHECK_THROWS_AS(coordinate.SetJson("{"), InvalidJSON);
    CHECK_THROWS_AS(coordinate.SetJson("not-json"), InvalidJSON);
    CHECK_THROWS_AS(coordinate.SetJson(R"({"X": 1,)"), InvalidJSON);
}

// The public JsonCpp interface must remain functional during migration.
TEST_CASE("Coordinate JsonValue legacy interface",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(3.0, 4.0);

    Json::Value values = coordinate.JsonValue();

    CHECK(values["X"].asDouble() == Approx(3.0));
    CHECK(values["Y"].asDouble() == Approx(4.0));

    Json::Value update(Json::objectValue);
    update["X"] = 9.5;

    coordinate.SetJsonValue(update);

    CHECK(coordinate.X == Approx(9.5));
    CHECK(coordinate.Y == Approx(4.0));
}

// Preserve JsonCpp 1.9.6 behavior: numeric strings are not doubles.
TEST_CASE("Coordinate JSON rejects numeric strings",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(1.0, 2.0);

    CHECK_THROWS(coordinate.SetJson(R"({"X":"12.5","Y":"-3.75"})"));

    CHECK(coordinate.X == Approx(1.0));
    CHECK(coordinate.Y == Approx(2.0));
}

// Exercise boolean conversion through the legacy JsonCpp boundary.
TEST_CASE("Coordinate JSON boolean conversion",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(5.0, 6.0);

    coordinate.SetJson(R"({"X":true,"Y":false})");

    CHECK(coordinate.X == Approx(1.0));
    CHECK(coordinate.Y == Approx(0.0));
}

// Coordinate JSON documents must be objects.
TEST_CASE("Coordinate JSON rejects non-object roots",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(1.0, 2.0);

    CHECK_THROWS_AS(coordinate.SetJson("[]"), InvalidJSON);
    CHECK_THROWS_AS(coordinate.SetJson("[1,2]"), InvalidJSON);
}

// A JSON null root leaves both coordinates unchanged in JsonCpp.
TEST_CASE("Coordinate JSON accepts null root",
          "[libopenshot][coordinate][json]")
{
    Coordinate coordinate(5.0, 6.0);

    CHECK_NOTHROW(coordinate.SetJson("null"));
    CHECK_NOTHROW(coordinate.SetJson("  null  "));
    CHECK_NOTHROW(coordinate.SetJson("\nnull\n"));

    CHECK(coordinate.X == Approx(5.0));
    CHECK(coordinate.Y == Approx(6.0));

    CHECK_THROWS_AS(coordinate.SetJson("not-json"), InvalidJSON);
}
