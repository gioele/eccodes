/*
 * (C) Copyright 2005- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */

#include <map>
#include <stdexcept>
#include <utility>
#include <limits>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <regex>

#include "step_unit.h"
#include "step.h"

Step step_from_string(std::string step)
{
    std::regex re("([0-9.]+)([smhDMYC]?)");
    std::smatch match;
    if (std::regex_match(step, match, re)) {
        if (match.size() == 3) {
            std::string value = match[1];
            std::string unit = match[2];
            if (unit.size() == 0) {
                unit = "h";
            }
            Step ret{std::stod(value), Unit{unit}};
            return ret;
        }
    }
    throw std::runtime_error("Could not parse step: " + step);
}

std::vector<Step> parse_range(const std::string& range_str)
{
    std::vector<Step> steps;
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = range_str.find("-", prev)) != std::string::npos) {
        steps.push_back(step_from_string(range_str.substr(prev, pos - prev)));
        prev = pos + 1;
    }
    steps.push_back(step_from_string(range_str.substr(prev)));
    return steps;
}

bool Step::operator==(const Step& other) const
{
    if ((internal_value_ == other.internal_value_) && (internal_unit_ == other.internal_unit_)) {
        return true;
    }
    return false;
}

bool Step::operator>(const Step& step) const
{
    auto [a, b] = find_common_units(this->copy().optimize_unit(), step.copy().optimize_unit());
    assert(a.internal_unit_ == b.internal_unit_);
    return a.internal_value_ > b.internal_value_;
}

bool Step::operator<(const Step& step) const
{
    auto [a, b] = find_common_units(this->copy().optimize_unit(), step.copy().optimize_unit());
    assert(a.internal_unit_ == b.internal_unit_);
    return a.internal_value_ < b.internal_value_;
}

Step Step::operator+(const Step& step) const
{
    auto [a, b] = find_common_units(this->copy().optimize_unit(), step.copy().optimize_unit());
    assert(a.internal_unit_ == b.internal_unit_);
    return Step(a.internal_value_ + b.internal_value_, a.internal_unit_);
}

Step Step::operator-(const Step& step) const
{
    auto [a, b] = find_common_units(this->copy().optimize_unit(), step.copy().optimize_unit());
    assert(a.internal_unit_ == b.internal_unit_);
    return Step(a.internal_value_ - b.internal_value_, a.internal_unit_);
}

std::pair<Step, Step> find_common_units(const Step& startStep, const Step& endStep)
{
    Step a = startStep;
    Step b = endStep;

    if (a.internal_value_ == 0 && b.internal_value_ == 0) {
        Unit unit = a.internal_unit_ > b.internal_unit_ ? a.internal_unit_ : b.internal_unit_;
        b.internal_unit_ = unit;
        b.unit_ = unit;
        a.internal_unit_ = unit;
        a.unit_ = unit;
    }
    else if (b.internal_value_ == 0) {
        b.internal_unit_ = a.internal_unit_;
        b.unit_ = a.internal_unit_;
        a.unit_ = a.internal_unit_;
        a.recalculateValue();
    }
    else if (a.internal_value_ == 0) {
        a.internal_unit_ = b.internal_unit_;
        a.unit_ = b.internal_unit_;
        b.unit_ = b.internal_unit_;
        b.recalculateValue();
    }
    else {
        auto it = std::find_if(Unit::grib_selected_units.begin(), Unit::grib_selected_units.end(), [&](const auto& e) {
            return e == a.unit().value<Unit::Value>() || e == b.unit().value<Unit::Value>();
        });

        assert(it != Unit::grib_selected_units.end());

        a.set_unit(*it);
        b.set_unit(*it);
        a.recalculateValue();
        b.recalculateValue();
        assert(a.internal_unit_ == b.internal_unit_);
    }

    return {a, b};
}

void Step::init_long(long value, const Unit& unit)
{
    internal_value_ = value;
    internal_unit_ = unit;
    unit_ = internal_unit_;
}

void Step::init_double(double value, const Unit& unit)
{
    auto seconds = Unit::get_converter().unit_to_duration(unit.value<Unit::Value>());
    init_long(static_cast<long>(value * seconds), Unit{Unit::Value::SECOND});
    unit_ = unit;
}

Step& Step::optimize_unit()
{
    if (internal_value_ == 0) {
        if (unit() > Unit{Unit::Value::HOUR}) {
            set_unit(Unit{Unit::Value::HOUR});
        }
        return *this;
    }

    unit_ = internal_unit_;
    Seconds<long> seconds = to_seconds<long>(internal_value_, internal_unit_);
    long abs_seconds = seconds.count() < 0 ? -seconds.count() : seconds.count();

    for (auto it = Unit::grib_selected_units.rbegin(); it != Unit::grib_selected_units.rend(); ++it) {
        long multiplier = Unit::get_converter().unit_to_duration(*it);
        if (abs_seconds % multiplier == 0) {
            internal_value_ = seconds.count() / multiplier;
            internal_unit_ = *it;
            unit_ = *it;
            return *this;
        }
    }

    return *this;
}

template <>
std::string Step::value<std::string>(const std::string& format) const {
    constexpr int MAX_SIZE = 128;
    char output[MAX_SIZE];
    std::string u;

    // Do not print unit if it is HOUR to keep backward compatibility
    // with previous versions of ecCodes (see ECC-1620). This is a temporary solution.
    //
    // TODO(maee): Remove this code to enable future output, e.g., 15h.

    if (unit_ != Unit::Value::HOUR)
        u =  unit_.value<std::string>();

    int err = snprintf(output, MAX_SIZE, (format + "%s").c_str(), value<double>(), u.c_str());
    if (err < 0 || err >= MAX_SIZE) {
        throw std::runtime_error("Error while formatting Step to string");
    }
    return output;
}
