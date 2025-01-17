#pragma once

#include "extra.h"

#include <cstdint>

#include "json/json.hpp"


struct ExtraHealth : Extra
{
   ExtraHealth();

   void reset();

   void addHealth(int32_t health);

   int32_t _life_count = 5;

   int32_t _health = 12;
   int32_t _health_max = 12;
};


void to_json(nlohmann::json& j, const ExtraHealth& d);
void from_json(const nlohmann::json& j, ExtraHealth& d);

