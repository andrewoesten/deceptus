#include "stenciltilemap.h"

#include <SFML/OpenGL.hpp>

#include "framework/tools/log.h"
#include "framework/tmxparser/tmxlayer.h"
#include "framework/tmxparser/tmxproperties.h"
#include "framework/tmxparser/tmxproperty.h"


bool StencilTileMap::load(TmxLayer* layer, TmxTileSet* tileset, const std::filesystem::path& base_path)
{
   TileMap::load(layer, tileset, base_path);

   if (!layer->_properties)
   {
      Log::Error() << "stencil layer does not have any properties";
      return false;
   }

   const auto it = layer->_properties->_map.find("stencil_reference");
   if (it == layer->_properties->_map.end())
   {
      Log::Error() << "stencil layer does not have 'stencil_reference' property";
      return false;
   }

   _stencil_reference = (*it).second->_value_string.value();

   return true;
}


void StencilTileMap::draw(sf::RenderTarget& color, sf::RenderTarget& normal, sf::RenderStates states) const
{
   prepareWriteToStencilBuffer();
   _stencil_tilemap->draw(color, normal, states);

   prepareWriteColor();
   TileMap::draw(color, normal, states);

   disableStencilTest();
}


void StencilTileMap::prepareWriteToStencilBuffer() const
{
   glClear(GL_STENCIL_BUFFER_BIT);
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS, 1, 1);
   glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
}


void StencilTileMap::prepareWriteColor() const
{
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glStencilFunc(GL_EQUAL, 0, 1);
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}


void StencilTileMap::disableStencilTest() const
{
   glDisable(GL_STENCIL_TEST);
}
