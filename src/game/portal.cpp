#include "portal.h"

// game
#include "constants.h"
#include "player.h"
#include "fixturenode.h"
#include "sfmlmath.h"

#include "tmxparser/tmximage.h"
#include "tmxparser/tmxlayer.h"
#include "tmxparser/tmxobject.h"
#include "tmxparser/tmxpolyline.h"
#include "tmxparser/tmxproperty.h"
#include "tmxparser/tmxproperties.h"
#include "tmxparser/tmxtileset.h"


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::Portal
 */
Portal::Portal()
 : mHeight(0),
   mZ(0),
   mDst(nullptr)
{
}



//-----------------------------------------------------------------------------
/*!
 * \brief Portal::draw
 */
void Portal::draw(sf::RenderTarget& window)
{
   for (const auto& sprite : mSprites)
   {
      window.draw(sprite);
   }
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::getPortalPosition
 * \return
 */
sf::Vector2f Portal::getPortalPosition()
{
   sf::Vector2f portalPos = mSprites.at(mSprites.size()-1).getPosition();
   return portalPos;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::getDst
 * \return
 */
Portal *Portal::getDst() const
{
   return mDst;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::setDst
 * \param dst
 */
void Portal::setDst(Portal *dst)
{
   mDst = dst;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::update
 * \param dt
 */
void Portal::update(float /*dt*/)
{
   sf::Vector2f playerPos = Player::getPlayer(0)->getPixelPosition();
   sf::Vector2f PortalPos = getPortalPosition();

   sf::Vector2f a(playerPos.x, playerPos.y);
   sf::Vector2f b(PortalPos.x + TILE_WIDTH * 0.5f, PortalPos.y);

   float distance = SfmlMath::length(a - b);
   bool atPortal = (distance < TILE_WIDTH * 1.0f);

   setPlayerAtPortal(atPortal);

   int i = 0;
   for (auto& sprite : mSprites)
   {
      sprite.setColor(
         sf::Color(
            255,
            255, // atPortal ? 150 : 255,
            255  // atPortal ? 150 : 255
         )
      );

      int x = static_cast<int>(mTilePosition.x);
      int y = static_cast<int>(mTilePosition.y);

      sprite.setPosition(
         sf::Vector2f(
            static_cast<float>(x * TILE_WIDTH),
            static_cast<float>((i + y) * TILE_HEIGHT)
         )
      );

      i++;
   }
}


//-----------------------------------------------------------------------------
/*!
 * \brief Level::link
 * \param srcdst
 */
void Portal::link(
   std::vector<Portal *> &portals,
   TmxObject* tmxObject
)
{
   std::vector<sf::Vector2f> srcdst = tmxObject->mPolyLine->mPolyLine;

   sf::Vector2f srcf = srcdst.at(0);
   sf::Vector2f dstf = srcdst.at(1);
   sf::Vector2i src(static_cast<int>(srcf.x), static_cast<int>(srcf.y));
   sf::Vector2i dst(static_cast<int>(dstf.x), static_cast<int>(dstf.y));

   int srcX = (int)(src.x + tmxObject->mX) / TILE_WIDTH;
   int srcY = (int)(src.y + tmxObject->mY) / TILE_HEIGHT;
   int dstX = (int)(dst.x + tmxObject->mX) / TILE_WIDTH;
   int dstY = (int)(dst.y + tmxObject->mY) / TILE_HEIGHT;

   Portal* srcPortal = nullptr;
   Portal* dstPortal = nullptr;

   for (auto portal : portals)
   {
      sf::Vector2f portalPos = portal->getPortalPosition();
      int px = (int)(portalPos.x / TILE_WIDTH);
      int py = (int)(portalPos.y / TILE_HEIGHT);

      if (px == srcX && py == srcY)
      {
         srcPortal = portal;
      }

      if (px == dstX && py == dstY)
      {
         dstPortal = portal;
      }

      if (srcPortal != nullptr && dstPortal != nullptr)
      {
         srcPortal->mDst = dstPortal;
         break;
      }
   }
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::addSprite
 * \param sprite
 */
void Portal::addSprite(const sf::Sprite & sprite)
{
   mSprites.push_back(sprite);
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::getZ
 * \return
 */
int Portal::getZ() const
{
   return mZ;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::setZ
 * \param z
 */
void Portal::setZ(int z)
{
   mZ = z;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::isPlayerAtPortal
 * \return
 */
bool Portal::isPlayerAtPortal() const
{
   return mPlayerAtPortal;
}



//-----------------------------------------------------------------------------
/*!
 * \brief Portal::setPlayerAtPortal
 * \param playerAtPortal
 */
void Portal::setPlayerAtPortal(bool playerAtPortal)
{
   mPlayerAtPortal = playerAtPortal;
}


//-----------------------------------------------------------------------------
/*!
 * \brief Portal::load
 * \param layer
 * \param tileSet
 * \return
 */
std::vector<Portal *> Portal::load(
   TmxLayer* layer,
   TmxTileSet* tileSet,
   const std::filesystem::path& basePath,
   const std::shared_ptr<b2World>&
)
{
   std::vector<Portal*> portals;

   sf::Vector2u tilesize = sf::Vector2u(tileSet->mTileWidth, tileSet->mTileHeight);
   const int *tiles      = layer->mData;
   unsigned int width    = layer->mWidth;
   unsigned int height   = layer->mHeight;
   unsigned int firstId  = tileSet->mFirstGid;

   // populate the vertex array, with one quad per tile
   for (unsigned int i = 0; i < width; ++i)
   {
      for (unsigned int j = 0; j < height; ++j)
      {
         // get the current tile number
         int tileNumber = tiles[i + j * width];

         if (tileNumber != 0)
         {
            // find matching Portal
            Portal* portal = nullptr;
            for (Portal* tmp : portals)
            {
               if (tmp->mTilePosition.x == i && tmp->mTilePosition.y + 1 == j )
               {
                  portal = tmp;
                  break;
               }
            }

            if (portal == nullptr)
            {
               portal = new Portal();
               portals.push_back(portal);
               portal->mTilePosition.x = static_cast<float>(i);
               portal->mTilePosition.y = static_cast<float>(j);
               portal->mTexture.loadFromFile((basePath / tileSet->mImage->mSource).string());

               if (layer->mProperties != nullptr)
               {
                  portal->setZ(layer->mProperties->mMap["z"]->mValueInt);
               }
            }

            portal->mHeight++;

            int tu = (tileNumber - firstId) % (portal->mTexture.getSize().x / tilesize.x);
            int tv = (tileNumber - firstId) / (portal->mTexture.getSize().x / tilesize.x);

            sf::Sprite sprite;
            sprite.setTexture(portal->mTexture);
            sprite.setTextureRect(
               sf::IntRect(
                  tu * TILE_WIDTH,
                  tv * TILE_HEIGHT,
                  TILE_WIDTH,
                  TILE_HEIGHT
               )
            );

            sprite.setPosition(
               sf::Vector2f(
                  static_cast<float>(i * TILE_WIDTH),
                  static_cast<float>(j * TILE_HEIGHT)
               )
            );

            portal->addSprite(sprite);
         }
      }
   }

   return portals;
}


