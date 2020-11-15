#pragma once

#include "tmxelement.h"

#include "SFML/Graphics.hpp"

struct TmxPolygon : TmxElement
{

  TmxPolygon() = default;
  void deserialize(tinyxml2::XMLElement* element);
  std::vector<sf::Vector2f> mPolyLine;
};

