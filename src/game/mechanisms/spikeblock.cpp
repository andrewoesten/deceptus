#include "spikeblock.h"

#include "framework/tmxparser/tmxobject.h"
#include "framework/tmxparser/tmxproperties.h"
#include "framework/tmxparser/tmxproperty.h"
#include "texturepool.h"
#include "player/player.h"


/*
   000
  +---+---+---+---+---+---+---+---+
  |   |   | . | . | . | o | o | * | enabled
  +---+---+---+---+---+---+---+---+
  | * | * | * | O | O |( )|( )|(#)|
  +---+---+---+---+---+---+---+---+
  |(#)|(#)|(#)|(#)|(#)|(#)|(#)|(#)|
  +---+---+---+---+---+---+---+---+
  |(#)|(#)|(=)|(=)|(=)|(=)|(=)|(=)| active
  +---+---+---+---+---+---+---+---+
  |( )| O | O | * | * | o | . |   | disabled
  +---+---+---+---+---+---+---+---+
                               039

*/

namespace
{
constexpr auto count_columns = 8;
constexpr auto animation_speed = 40.0f;
constexpr auto damage = 100;
}


SpikeBlock::SpikeBlock(GameNode* parent)
 : GameNode(parent)
{
   setClassName(typeid(SpikeBlock).name());
}


void SpikeBlock::setup(TmxObject* tmx_object)
{
   setObjectName(tmx_object->_name);

   _texture_map = TexturePool::getInstance().get("data/sprites/enemy_spikeblock.png");
   _sprite.setTexture(*_texture_map);
   _sprite.setPosition(tmx_object->_x_px, tmx_object->_y_px);

   _rectangle = {
      static_cast<int32_t>(tmx_object->_x_px),
      static_cast<int32_t>(tmx_object->_y_px),
      static_cast<int32_t>(tmx_object->_width_px),
      static_cast<int32_t>(tmx_object->_height_px)
   };

   setZ(static_cast<int32_t>(ZDepth::ForegroundMin) + 1);

   if (tmx_object->_properties)
   {
      const auto z_it = tmx_object->_properties->_map.find("z");
      if (z_it != tmx_object->_properties->_map.end())
      {
         const auto z_index = static_cast<uint32_t>(z_it->second->_value_int.value());
         setZ(z_index);
      }

      const auto enabled_it = tmx_object->_properties->_map.find("enabled");
      if (enabled_it != tmx_object->_properties->_map.end())
      {
         const auto enabled = static_cast<bool>(enabled_it->second->_value_bool.value());
         setEnabled(enabled);
      }

      const auto mode_it = tmx_object->_properties->_map.find("mode");
      if (mode_it != tmx_object->_properties->_map.end())
      {
         auto mode_str = static_cast<std::string>(mode_it->second->_value_string.value());
         if (mode_str == "interval")
         {
            _mode = Mode::Interval;
         }
      }

      const auto time_on_it = tmx_object->_properties->_map.find("time_on_ms");
      if (time_on_it != tmx_object->_properties->_map.end())
      {
         _time_on_ms = static_cast<int32_t>(time_on_it->second->_value_int.value());
      }

      const auto time_off_it = tmx_object->_properties->_map.find("time_off_ms");
      if (time_on_it != tmx_object->_properties->_map.end())
      {
         _time_off_ms = static_cast<int32_t>(time_off_it->second->_value_int.value());
      }
   }

   updateSpriteRect();
}


void SpikeBlock::updateSpriteRect()
{
   _tu_tl = _sprite_index_current % count_columns;
   _tv_tl = _sprite_index_current / count_columns;

   _sprite.setTextureRect({
      _tu_tl * PIXELS_PER_TILE,
      _tv_tl * PIXELS_PER_TILE,
      PIXELS_PER_TILE,
      PIXELS_PER_TILE}
   );
}


const sf::IntRect& SpikeBlock::getPixelRect() const
{
   return _rectangle;
}


void SpikeBlock::draw(sf::RenderTarget& target, sf::RenderTarget& /*normal*/)
{
   target.draw(_sprite);
}


void SpikeBlock::update(const sf::Time& dt)
{
   if (_mode == Mode::Interval)
   {
      _elapsed += dt;

      if (!isEnabled() && _elapsed.asMilliseconds() >= _time_off_ms)
      {
         setEnabled(true);
         _elapsed = {};
      }
      else if (isEnabled() && _elapsed.asMilliseconds() >= _time_on_ms)
      {
         setEnabled(false);
         _elapsed = {};
      }
   }

   if (Player::getCurrent()->getPlayerPixelRect().intersects(_rectangle))
   {
      if (_sprite_index_current >= _sprite_index_deadly_min && _sprite_index_current <= _sprite_index_deadly_max)
      {
         Player::getCurrent()->damage(damage);
      }
   }

   if (_sprite_index_current != _sprite_index_target)
   {
      _sprite_value += dt.asSeconds() * animation_speed;

      const auto sprite_index = static_cast<int32_t>(std::floor(_sprite_value));

      if (sprite_index != _sprite_index_current)
      {
         // reset after completing animation cycle
         if (sprite_index >= _sprite_index_disabled)
         {
            _sprite_value = 0.0f;
            _sprite_index_current = 0;
            _sprite_index_target = 0;
         }
         else
         {
            _sprite_index_current = sprite_index;
         }

         updateSpriteRect();
      }
   }
}


void SpikeBlock::setEnabled(bool enabled)
{
   GameMechanism::setEnabled(enabled);
   _sprite_index_target = (enabled ? _sprite_index_enabled : _sprite_index_disabled);
}


