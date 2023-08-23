#include <map>
#include <stdexcept>
#include <utility>
#include <limits>
#include <iostream>
#include <algorithm>
#include <cassert>

#include "step.h"


UnitType::Map UnitType::map_{};

std::vector<Unit> UnitType::unitOrder = {
            Unit::SECOND,
            Unit::MINUTE,
            Unit::HOUR,
        };


std::string parse_step(std::string step) {
    if (step.find_first_of("smhdMYC") == std::string::npos) {
        step += "h";
    }
    return step;
}


std::pair<Step<long>, Step<long>> findCommonUnits(const Step<long>& startStep, const Step<long>& endStep) {
    Step<long> a = startStep;
    Step<long> b = endStep;

    if (a.value_ == 0 || b.value_ == 0) {
        if (a.value_ == 0 && b.value_ == 0) {
            UnitType unit = a.unit_ > b.unit_ ? a.unit_ : b.unit_;
            a.setUnit(unit);
            b.setUnit(unit);
        }
        else if (a.value_ == 0) {
            a.setUnit(b.unit_);
        }
        else if (b.value_ == 0) {
            b.setUnit(a.unit_);
        }
        return {a, b};
    }

    auto it = std::find_if(UnitType::unitOrder.begin(), UnitType::unitOrder.end(), [&](const auto& e) {
        return e == a.unit().to_value() || e == b.unit().to_value();
    });

    assert(it != UnitType::unitOrder.end());

    a.setUnit(*it);
    b.setUnit(*it);

    return {a, b};
}
