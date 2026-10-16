/**
 * @file
 * @brief Callback-driven Qt image reader
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef OPENSHOT_QTPAINTREADER_H
#define OPENSHOT_QTPAINTREADER_H

#include "ReaderBase.h"
#include "Fraction.h"

#include <QImage>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace openshot {

class QtPaintReader : public ReaderBase {
public:
    using RenderCallback = std::function<QImage(int64_t)>;

private:
    RenderCallback renderer;
    bool is_open = false;

public:
    QtPaintReader(
        RenderCallback callback,
        int width,
        int height,
        Fraction fps = Fraction(30, 1),
        double duration = 30.0);

    ~QtPaintReader() override;

    void Open() override;
    void Close() override;
    bool IsOpen() override;

    // Frame numbers are 1-based. Requests beyond the declared
    // duration are permitted, matching QtImageReader behavior.
    std::shared_ptr<Frame> GetFrame(int64_t number) override;

    CacheBase* GetCache() override { return nullptr; }

    std::string Name() override { return "QtPaintReader"; }

    std::string Json() const override;
    Json::Value JsonValue() const override;

    void SetJson(const std::string value) override;
    void SetJsonValue(const Json::Value root) override;
};

}

#endif
