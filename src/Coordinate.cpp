/**
 * @file
 * @brief Source file for Coordinate class
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @ref License
 */

// Copyright (c) 2008-2019 OpenShot Studios, LLC
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Coordinate.h"
#include "QtJsonAdapter.h"
#include <QJsonObject>
#include <QJsonValue>
#include "Exceptions.h"

using namespace openshot;

// Default constructor for a coordinate, delegating to the full signature
Coordinate::Coordinate() : Coordinate::Coordinate(0, 0) {}

// Constructor which also allows the user to set the X and Y
Coordinate::Coordinate(double x, double y) : X(x), Y(y) {}

// Constructor which accepts a std::pair for (X, Y)
Coordinate::Coordinate(const std::pair<double, double>& co)
	: X(co.first), Y(co.second) {}

// Generate JSON string of this object
std::string Coordinate::Json() const {

	// Return formatted string
	return JsonValue().toStyledString();
}

// Generate Json::Value for this object
Json::Value Coordinate::JsonValue() const {

	// Create root json object
	Json::Value root;
	root["X"] = X;
	root["Y"] = Y;
	//root["increasing"] = increasing;
	//root["repeated"] = Json::Value(Json::objectValue);
	//root["repeated"]["num"] = repeated.num;
	//root["repeated"]["den"] = repeated.den;
	//root["delta"] = delta;

	// return JsonValue
	return root;
}

// Load JSON string into this object
void Coordinate::SetJson(const std::string value) {

    const std::size_t first =
        value.find_first_not_of(" \t\r\n");

    if (first != std::string::npos) {
        const std::size_t last =
            value.find_last_not_of(" \t\r\n");

        if (value.substr(first, last - first + 1) == "null")
            return;
    }

    try {
        const QJsonDocument document =
            openshot::detail::parseQtJson(value);

        if (!document.isObject()) {
            throw InvalidJSON("Coordinate JSON must be an object");
        }

        const QJsonObject object = document.object();

        Json::Value root(Json::objectValue);

        if (object.contains("X") && !object.value("X").isNull()) {
            const QJsonValue x = object.value("X");
            if (x.isDouble())
                root["X"] = x.toDouble();
            else if (x.isBool())
                root["X"] = x.toBool();
            else if (x.isString())
                root["X"] = x.toString().toStdString();
        }

        if (object.contains("Y") && !object.value("Y").isNull()) {
            const QJsonValue y = object.value("Y");
            if (y.isDouble())
                root["Y"] = y.toDouble();
            else if (y.isBool())
                root["Y"] = y.toBool();
            else if (y.isString())
                root["Y"] = y.toString().toStdString();
        }

        SetJsonValue(root);
    }
    catch (const std::exception&) {
        throw InvalidJSON(
            "JSON is invalid (missing keys or invalid data types)");
    }
}

void Coordinate::SetJsonValue(const Json::Value root) {

	// Set data from Json (if key is found)
	if (!root["X"].isNull())
		X = root["X"].asDouble();
	if (!root["Y"].isNull())
		Y = root["Y"].asDouble();
}
