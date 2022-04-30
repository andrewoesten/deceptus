#include "rotatingblade.h"

#include "framework/tmxparser/tmxpolygon.h"
#include "framework/tmxparser/tmxpolyline.h"
#include "framework/tools/log.h"
#include "game/texturepool.h"


RotatingBlade::RotatingBlade(GameNode* parent)
 : GameNode(parent)
{
   setClassName(typeid(RotatingBlade).name());

   setZ(30);

   _texture_map = TexturePool::getInstance().get("data/sprites/enemy_rotating_blade.png");
   _sprite.setTexture(*_texture_map.get());
}


void RotatingBlade::setup(const GameDeserializeData& data)
{
   if (!data._tmx_object->_polygon && !data._tmx_object->_polyline)
   {
      Log::Error() << "the tmx object is neither a polygon or polyline";
      return;
   }

   _path = data._tmx_object->_polygon ? data._tmx_object->_polygon->_polyline : data._tmx_object->_polyline->_polyline;
   _path_type = data._tmx_object->_polygon ? PathType::Polygon : PathType::Polyline;

   std::transform(_path.begin(), _path.end(), _path.begin(), [data](auto& vec){
       return vec + sf::Vector2f{data._tmx_object->_x_px, data._tmx_object->_y_px};
   });

   _path_interpolation.addKeys(_path);
}


void RotatingBlade::update(const sf::Time& dt)
{
   if (_enabled)
   {
      _velocity = std::min<float>(1.0f, _velocity + _settings._blade_acceleration);
   }
   else
   {
      _velocity = std::max<float>(0.0f, _velocity - _settings._blade_deceleration);
   }

   // exit early if velocity is under a certain threshold
   const auto movement_delta = dt.asSeconds() * _velocity * _settings._movement_speed;
   if (_path_type == PathType::Polyline)
   {
      _path_interpolation.updateTime(movement_delta);
   }
   else
   {
      _path_interpolation.updateTimeLooped(movement_delta);
   }

   _angle += dt.asSeconds() * _velocity * _direction * _settings._blade_rotation_speed;
   const auto pos = _path_interpolation.computePosition(_path_interpolation.getTime());

   _sprite.setRotation(_angle);
   _sprite.setPosition(pos);
}


void RotatingBlade::draw(sf::RenderTarget& target, sf::RenderTarget& /*normal*/)
{
   target.draw(_sprite);
}


void RotatingBlade::setEnabled(bool /*enabled*/)
{

}
