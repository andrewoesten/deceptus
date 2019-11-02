#include "spikes.h"

#include "constants.h"
#include "player.h"
#include "texturepool.h"

#include "tmxparser/tmxlayer.h"
#include "tmxparser/tmxtileset.h"
#include "tmxparser/tmxproperty.h"
#include "tmxparser/tmxproperties.h"

#include <iostream>


sf::Texture Spikes::sTexture;
#define SPIKES_PER_ROW 22


namespace
{
static const auto updateTimeUpMs = 5;
static const auto updateTimeDownMs = 30;
static const auto downTime = 2000;
static const auto upTime = 2000;
}


void Spikes::draw(sf::RenderTarget& window)
{
   window.draw(mSprite);
}


void Spikes::updateInterval()
{
   auto wait = false;

   if (mTu == 0)
   {
      mTriggered = false;

      if (mElapsedMs < upTime)
      {
         wait = true;
      }
      else
      {
         // skip the first few frames on the way back..
         mTu = 7;
      }
   }

   if (mTu == SPIKES_PER_ROW - 1)
   {
      mTriggered = true;

      if (mElapsedMs < downTime)
      {
         wait = true;
      }
   }

   const auto updateTime = (mTriggered ? updateTimeUpMs : updateTimeDownMs);
   if (!wait && mElapsedMs > updateTime)
   {
      mElapsedMs = (mElapsedMs % updateTime);

      if (mTriggered)
      {
         // extract
         mTu-=2;
         if (mTu < 0)
         {
            mTu = 0;
         }
      }
      else
      {
         // retract
         mTu++;
         if (mTu >= SPIKES_PER_ROW)
         {
            mTu = SPIKES_PER_ROW - 1;
         }
      }
   }

   mDeadly = (mTu < 10);
}


void Spikes::update(const sf::Time& dt)
{
   mElapsedMs += dt.asMilliseconds();

   switch (mMode)
   {
      case Mode::Trap:
      {
         break;
      }
      case Mode::Interval:
      {
         updateInterval();
         break;
      }
      case Mode::Toggled:
      {
         break;
      }
      case Mode::Invalid:
      {
         break;
      }
   }

   mSprite.setTextureRect({mTu * PIXELS_PER_TILE, mTv * PIXELS_PER_TILE, PIXELS_PER_TILE, PIXELS_PER_TILE});

   if (mDeadly)
   {
      // check for intersection with player
      auto playerRect = Player::getCurrent()->getPlayerPixelRect();
      if (playerRect.intersects(mRect))
      {
         Player::getCurrent()->damage(100);
      }
   }
}


std::vector<std::shared_ptr<Spikes> > Spikes::load(
   TmxLayer* layer,
   TmxTileSet* tileSet,
   const std::filesystem::path& basePath,
   const std::shared_ptr<b2World>& /*world*/
)
{
   sTexture = *TexturePool::getInstance().get(basePath / "tilesets" / "spikes.png");

   std::vector<std::shared_ptr<Spikes>> allSpikes;

   const auto tiles    = layer->mData;
   const auto width    = layer->mWidth;
   const auto height   = layer->mHeight;
   const auto firstId  = tileSet->mFirstGid;

   const int32_t tilesPerRow = sTexture.getSize().x / PIXELS_PER_TILE;

   for (auto i = 0u; i < width; ++i)
   {
      for (auto j = 0u; j < height; ++j)
      {
         auto tileNumber = tiles[i + j * width];

         if (tileNumber != 0)
         {
            auto id = (tileNumber - firstId);
            auto spikes = std::make_shared<Spikes>();
            spikes->mMode = Mode::Interval;
            allSpikes.push_back(spikes);

            spikes->mTilePosition.x = static_cast<float>(i);
            spikes->mTilePosition.y = static_cast<float>(j);

            // std::cout << "look up: " << id << std::endl;

            spikes->mTu = static_cast<int32_t>(id % tilesPerRow);
            spikes->mTv = static_cast<int32_t>(id / tilesPerRow);

            if (layer->mProperties != nullptr)
            {
               spikes->setZ(layer->mProperties->mMap["z"]->mValueInt);
            }

            spikes->mRect = {
               static_cast<int32_t>(i * PIXELS_PER_TILE),
               static_cast<int32_t>(j * PIXELS_PER_TILE),
               PIXELS_PER_TILE,
               PIXELS_PER_TILE
            };

            sf::Sprite sprite;
            sprite.setTexture(sTexture);
            sprite.setPosition(
               sf::Vector2f(
                  static_cast<float>(i * PIXELS_PER_TILE),
                  static_cast<float>(j * PIXELS_PER_TILE)
               )
            );

            spikes->mSprite = sprite;
         }
      }
   }

   return allSpikes;
}


int32_t Spikes::getZ() const
{
   return mZ;
}


void Spikes::setZ(const int32_t& z)
{
   mZ = z;
}

