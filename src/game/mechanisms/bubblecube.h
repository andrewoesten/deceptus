#pragma once

//Bubble cube
//
//- You can stay on top of it as long as you want.
//- Once you jump off/ stop colliding with it, it vanishes "pop animation"
//- cube reappears after n seconds.
//- Basically is a 1-jump-platform before vanishes.

class GameNode;
struct TmxObject;

#include "fixturenode.h"
#include "gamemechanism.h"

#include "Box2D/Box2D.h"

#include <filesystem>


class BubbleCube : public FixtureNode, public GameMechanism
{

public:

   BubbleCube(
      GameNode* parent,
      const std::shared_ptr<b2World>& world,
      TmxObject* tmx_object,
      const std::filesystem::path& base_path
   );

   void draw(sf::RenderTarget& target, sf::RenderTarget& normal) override;
   void update(const sf::Time& dt) override;

   void beginContact();
   void endContact();


private:

   float _elapsed_s = 0.0f;
   bool _popped = false;
   int32_t _contact_count = 0;
   sf::Time _pop_time;

   // sf
   std::shared_ptr<sf::Texture> _texture;
   sf::Sprite _sprite;

   // b2d
   b2Body* _body = nullptr;
   b2Fixture* _fixture = nullptr;
   b2Vec2 _position_m;
   b2PolygonShape _shape;
};

