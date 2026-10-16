/**
 * @file
 * @brief Unit tests for openshot::QtPaintReader
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "openshot_catch.h"

#include "QtPaintReader.h"
#include "Exceptions.h"
#include "Frame.h"

#include <QColor>
#include <QImage>
#include <QPainter>

#include <stdexcept>

using namespace openshot;

TEST_CASE("QtPaintReader_Invalid_Arguments",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) {
        QImage image(64, 48, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        return image;
    };

    CHECK_THROWS_AS(QtPaintReader({}, 64, 48),
                    std::invalid_argument);
    CHECK_THROWS_AS(QtPaintReader(render, 0, 48),
                    std::invalid_argument);
    CHECK_THROWS_AS(QtPaintReader(render, 64, -1),
                    std::invalid_argument);
    CHECK_THROWS_AS(QtPaintReader(render, 64, 48,
                                 Fraction(0, 1)),
                    std::invalid_argument);
    CHECK_THROWS_AS(QtPaintReader(render, 64, 48,
                                 Fraction(30, 1), 0.0),
                    std::invalid_argument);
}

TEST_CASE("QtPaintReader_Lifecycle",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) {
        QImage image(64, 48, QImage::Format_ARGB32);
        image.fill(Qt::red);
        return image;
    };

    QtPaintReader reader(render, 64, 48);

    CHECK_FALSE(reader.IsOpen());
    CHECK_THROWS_AS(reader.GetFrame(1), ReaderClosed);

    reader.Open();
    CHECK(reader.IsOpen());

    CHECK_THROWS_AS(reader.GetFrame(0),
                    std::out_of_range);

    auto frame = reader.GetFrame(1);
    REQUIRE(frame != nullptr);
    CHECK(frame->GetWidth() == 64);
    CHECK(frame->GetHeight() == 48);

    reader.Close();
    CHECK_FALSE(reader.IsOpen());
    CHECK_THROWS_AS(reader.GetFrame(1), ReaderClosed);
}

TEST_CASE("QtPaintReader_Dynamic_Painting",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t number) {
        QImage image(64, 48, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.fillRect(8, 8, 24, 20,
                         number == 1 ? Qt::red : Qt::blue);
        painter.end();

        return image;
    };

    QtPaintReader reader(render, 64, 48);
    reader.Open();

    auto first = reader.GetFrame(1);
    auto second = reader.GetFrame(2);

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);

    auto image1 = first->GetImage();
    auto image2 = second->GetImage();

    REQUIRE(image1 != nullptr);
    REQUIRE(image2 != nullptr);

    CHECK(image1->pixelColor(12, 12) == QColor(Qt::red));
    CHECK(image2->pixelColor(12, 12) == QColor(Qt::blue));

    CHECK(image1->pixelColor(0, 0).alpha() == 0);
    CHECK(image2->pixelColor(0, 0).alpha() == 0);

    reader.Close();
}

TEST_CASE("QtPaintReader_Empty_Image",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) {
        return QImage();
    };

    QtPaintReader reader(render, 64, 48);
    reader.Open();

    CHECK_THROWS_AS(reader.GetFrame(1),
                    std::runtime_error);

    reader.Close();
}

TEST_CASE("QtPaintReader_Wrong_Dimensions",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) {
        QImage image(32, 24, QImage::Format_ARGB32);
        image.fill(Qt::green);
        return image;
    };

    QtPaintReader reader(render, 64, 48);
    reader.Open();

    CHECK_THROWS_AS(reader.GetFrame(1),
                    std::runtime_error);

    reader.Close();
}

TEST_CASE("QtPaintReader_Callback_Exception",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) -> QImage {
        throw std::runtime_error("Rendering failed");
    };

    QtPaintReader reader(render, 64, 48);
    reader.Open();

    CHECK_THROWS_AS(reader.GetFrame(1),
                    std::runtime_error);

    reader.Close();
}

TEST_CASE("QtPaintReader_Frame_Independence",
          "[libopenshot][qtpaintreader]")
{
    QImage source(64, 48, QImage::Format_ARGB32);
    source.fill(Qt::red);

    auto render = [&source](int64_t) {
        return source;
    };

    QtPaintReader reader(render, 64, 48);
    reader.Open();

    auto first = reader.GetFrame(1);
    REQUIRE(first != nullptr);

    source.fill(Qt::blue);

    auto second = reader.GetFrame(2);
    REQUIRE(second != nullptr);

    auto image1 = first->GetImage();
    auto image2 = second->GetImage();

    REQUIRE(image1 != nullptr);
    REQUIRE(image2 != nullptr);

    CHECK(image1->pixelColor(10, 10) == QColor(Qt::red));
    CHECK(image2->pixelColor(10, 10) == QColor(Qt::blue));

    reader.Close();
}

TEST_CASE("QtPaintReader_JSON_Behavior",
          "[libopenshot][qtpaintreader]")
{
    auto render = [](int64_t) {
        QImage image(64, 48, QImage::Format_ARGB32);
        image.fill(Qt::green);
        return image;
    };

    QtPaintReader reader(render, 64, 48);

    Json::Value json = reader.JsonValue();

    CHECK(json["type"].asString() == "QtPaintReader");
    CHECK_FALSE(json["callback_serializable"].asBool());

    CHECK_THROWS_AS(reader.SetJson("{}"),
                    std::logic_error);

    CHECK_THROWS_AS(
        reader.SetJsonValue(Json::Value(Json::objectValue)),
        std::logic_error);
}
