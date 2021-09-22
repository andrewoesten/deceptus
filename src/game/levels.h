#pragma once

#include <string>
#include <vector>

#include "json/json.hpp"


struct LevelItem
{
  std::string mLevelName;
};


void from_json(const nlohmann::json& j, LevelItem& item);


struct Levels
{
   Levels() = default;

   std::vector<LevelItem> _levels;

   void deserialize(const std::string& data);
   void deserializeFromFile(const std::string& filename = "data/config/levels.json");

   static Levels& getInstance();
};
