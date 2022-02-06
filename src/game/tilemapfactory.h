#pragma once

struct TmxLayer;

#include <memory>
#include <vector>
#include "tilemap.h"


namespace TileMapFactory
{
   std::shared_ptr<TileMap> makeTileMap(TmxLayer* layer);
   void merge(const std::vector<std::shared_ptr<TileMap>>& tile_maps);
}

