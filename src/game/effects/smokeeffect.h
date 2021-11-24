#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>


struct TmxObject;
struct TmxObjectGroup;

class SmokeEffect
{

public:

   SmokeEffect();

   void drawToZ(sf::RenderTarget& target, sf::RenderStates states, int z);
   void update(const sf::Time& time);

   static std::shared_ptr<SmokeEffect> deserialize(TmxObject* tmx_object, TmxObjectGroup* object_group);

private:

   std::shared_ptr<sf::Texture> _texture;

   struct SmokeParticle
   {
      sf::Sprite _sprite;
      float _rot = 0.0f;
      float _rot_dir = 1.0f;
      float _time_offset = 0.0f;

      sf::Vector2f _offset;
      sf::Vector2f _center;
   };

   std::vector<SmokeParticle> _particles;
   int32_t _z = 20;
   sf::Time _last_update_time;

   float _pixel_ratio = 1.0f;
   float _velocity = 1.0f;
   sf::Vector2u _size_px;
   sf::Vector2f _offset_px;
   sf::BlendMode _blend_mode = sf::BlendAdd;
};

