/**
 * @file
 * @brief Callback-driven Qt image reader
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "QtPaintReader.h"
#include "Exceptions.h"
#include "Frame.h"

#include <cmath>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace openshot {

QtPaintReader::QtPaintReader(
    RenderCallback callback,
    int width,
    int height,
    Fraction fps,
    double duration)
    : renderer(std::move(callback))
{
    if (!renderer)
        throw std::invalid_argument("QtPaintReader requires a callback");

    if (width <= 0 || height <= 0)
        throw std::invalid_argument("QtPaintReader requires positive dimensions");

    if (fps.num <= 0 || fps.den <= 0)
        throw std::invalid_argument("QtPaintReader requires positive FPS");

    if (!std::isfinite(duration) || duration <= 0.0)
        throw std::invalid_argument("QtPaintReader requires positive finite duration");

    info.has_video = true;
    info.has_audio = false;
    info.has_single_image = false;
    info.width = width;
    info.height = height;
    info.fps = fps;
    info.duration = duration;

    info.video_length = static_cast<int64_t>(
        std::ceil(duration * fps.ToDouble()));

    info.vcodec = "QImage";
    info.pixel_ratio = Fraction(1, 1);
    info.video_timebase = fps.Reciprocal();

    Fraction display(width, height);
    display.Reduce();
    info.display_ratio = display;

    info.sample_rate = 44100;
    info.channels = 2;
}

QtPaintReader::~QtPaintReader()
{
    Close();
}

void QtPaintReader::Open()
{
    const std::lock_guard<std::recursive_mutex> lock(getFrameMutex);
    is_open = true;
}

void QtPaintReader::Close()
{
    const std::lock_guard<std::recursive_mutex> lock(getFrameMutex);
    is_open = false;
}

bool QtPaintReader::IsOpen()
{
    const std::lock_guard<std::recursive_mutex> lock(getFrameMutex);
    return is_open;
}

std::shared_ptr<Frame> QtPaintReader::GetFrame(int64_t number)
{
    const std::lock_guard<std::recursive_mutex> lock(getFrameMutex);

    if (!is_open)
        throw ReaderClosed(
            "QtPaintReader is closed. Call Open() first.",
            "<callback>");

    if (number < 1)
        throw std::out_of_range("Frame numbers must start at 1");

    QImage rendered = renderer(number);

    if (rendered.isNull())
        throw std::runtime_error(
            "QtPaintReader callback returned an empty QImage");

    if (rendered.width() != info.width ||
        rendered.height() != info.height)
        throw std::runtime_error(
            "QtPaintReader callback returned incorrect dimensions");

    auto image = std::make_shared<QImage>(
        rendered.convertToFormat(QImage::Format_ARGB32).copy());

    const auto sample_count = Frame::GetSamplesPerFrame(
        number, info.fps, info.sample_rate, info.channels);

    auto frame = std::make_shared<Frame>(
        number,
        info.width,
        info.height,
        "#000000",
        sample_count,
        info.channels);

    frame->AddImage(image);

    return frame;
}

std::string QtPaintReader::Json() const
{
    return JsonValue().toStyledString();
}

Json::Value QtPaintReader::JsonValue() const
{
    Json::Value root = ReaderBase::JsonValue();
    root["type"] = "QtPaintReader";
    root["callback_serializable"] = false;
    return root;
}

void QtPaintReader::SetJson(const std::string)
{
    throw std::logic_error(
        "QtPaintReader callbacks cannot be restored from JSON");
}

void QtPaintReader::SetJsonValue(const Json::Value)
{
    throw std::logic_error(
        "QtPaintReader callbacks cannot be restored from JSON");
}

} // namespace openshot
