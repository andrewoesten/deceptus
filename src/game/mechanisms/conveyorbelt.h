#pragma once

#include <filesystem>

#include "constants.h"
#include "gamemechanism.h"
#include "fixturenode.h"
#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

struct TmxObject;

class ConveyorBelt : public FixtureNode, public GameMechanism
{

   public:

      ConveyorBelt(
         GameNode* parent,
         const std::shared_ptr<b2World>& world,
         TmxObject* tmxObject,
         const std::filesystem::path& basePath
      );

      b2Body *getBody() const;
      float getVelocity() const;
      void setVelocity(float velocity);

      void draw(sf::RenderTarget& target) override;
      void update(const sf::Time& dt) override;
      void setEnabled(bool enabled) override;

      sf::IntRect getPixelRect() const;


      static void update();
      static void processContact(b2Contact *contact);
      static void processFixtureNode(FixtureNode* fixtureNode, b2Body* collidingBody);


      void updateSprite();

   private:

      b2Body* mBody = nullptr;
      b2Vec2 mPositionB2d;
      sf::Vector2f mPositionSf;
      b2PolygonShape mShapeBounds;
      sf::IntRect mBeltPixelRect;
      sf::IntRect mArrowPixelRect;
      float mElapsed = 0.0f;
      bool mPointsRight = true;
      float mLeverLag = 1.0f;

      static sf::Texture sTexture;
      std::vector<sf::Sprite> mBeltSprites;
      std::vector<sf::Sprite> mArrowSprites;

      // bool mActive = true;
      float mVelocity = -0.2f;

      static std::vector<b2Body*> sBodiesOnBelt;
};

