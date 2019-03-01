#include "inventorylayer.h"

#include "gameconfiguration.h"
#include "gamecontrollerintegration.h"
#include "globalclock.h"
#include "joystick/gamecontroller.h"

#include "player.h"
#include "extramanager.h"
#include "inventoryitem.h"


namespace {
  const auto iconWidth  = 40;
  const auto iconHeight = 24;
  const auto quadWidth  = 38;
  const auto quadHeight = 38;
  const auto dist = 9.1f;
  const auto iconQuadDist = (iconWidth - quadWidth);
}


//---------------------------------------------------------------------------------------------------------------------
GameControllerInfo InventoryLayer::getJoystickInfo() const
{
  return mJoystickInfo;
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::setJoystickInfo(const GameControllerInfo &joystickInfo)
{
  mJoystickInfo = joystickInfo;
}


//---------------------------------------------------------------------------------------------------------------------
InventoryLayer::InventoryLayer()
{
  addDemoInventory();
  mInventuryTexture.loadFromFile("data/game/inventory.png");
  mCursorSprite.setTexture(mInventuryTexture);
  mCursorSprite.setTextureRect({0, 512-48, 48, 48});
  initializeController();
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::initializeController()
{
  if (GameControllerIntegration::getCount() > 0)
  {
     auto gji = GameControllerIntegration::getInstance(0);
     gji->getController()->addButtonPressedCallback(SDL_CONTROLLER_BUTTON_Y, [/*this*/](){/*jump();*/});
  }
}


//---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<InventoryItem>>* InventoryLayer::getInventory()
{
  return &Player::getPlayer(0)->getExtraManager()->mInventory;
}



//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::addItem(int32_t x, int32_t y, ItemType type)
{
  auto item = std::make_shared<InventoryItem>();
  item->mType = type;
  item->mSprite.setTexture(mInventuryTexture);
  item->mSprite.setTextureRect({x * iconWidth, y * iconHeight, iconWidth, iconHeight});
  getInventory()->push_back(item);
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::addDemoInventory()
{
  addItem(0,3 , ItemType::KeyCrypt);
  addItem(8,3 , ItemType::KeySkull);
  addItem(4,1 , ItemType::Shovel  );
  addItem(7,6 , ItemType::Knife   );
  addItem(9,6 , ItemType::Saw     );
  addItem(9,7 , ItemType::Crowbar );
  addItem(0,8 , ItemType::Dynamite);
  addItem(5,9 , ItemType::Money   );
  addItem(0,11, ItemType::Match   );
  addItem(4,12, ItemType::LoveBomb);
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::draw(sf::RenderTarget &window)
{
  auto w = GameConfiguration::getInstance().mViewWidth;
  auto h = GameConfiguration::getInstance().mViewHeight;

  sf::View view(sf::FloatRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)));
  window.setView(view);

  const sf::Color color = {50, 70, 100, 150};

  auto x = dist;
  auto y = 220;

  for (int i = 0; i < 12; i++)
  {
    sf::Vertex quad[] =
    {
       sf::Vertex(sf::Vector2f(static_cast<float>(x),                                 static_cast<float>(y)                                 ), color),
       sf::Vertex(sf::Vector2f(static_cast<float>(x),                                 static_cast<float>(y) + static_cast<float>(quadHeight)), color),
       sf::Vertex(sf::Vector2f(static_cast<float>(x) + static_cast<float>(quadWidth), static_cast<float>(y) + static_cast<float>(quadHeight)), color),
       sf::Vertex(sf::Vector2f(static_cast<float>(x) + static_cast<float>(quadWidth), static_cast<float>(y)                                 ), color)
    };
    window.draw(quad, 4, sf::Quads);
    x += quadWidth + dist;
  }

  y = 230;
  x = dist;

  for (auto item : Player::getPlayer(0)->getExtraManager()->mInventory)
  {
    item->mSprite.setPosition(static_cast<float>(x), static_cast<float>(y));
    window.draw(item->mSprite);
    x += iconWidth + dist - iconQuadDist;
  }

  mCursorPosition.y = 215.0f;
  mCursorSprite.setPosition(mCursorPosition);
  window.draw(mCursorSprite);
}


//---------------------------------------------------------------------------------------------------------------------
bool InventoryLayer::isControllerActionSkipped() const
{
  auto skipped = false;
  auto now = GlobalClock::getInstance()->getElapsedTimeInS();
  if (now - mJoystickUpdateTime < 0.3f)
  {
    skipped = true;
  }

  return skipped;
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::updateControllerActions()
{
  auto gji =GameControllerIntegration::getInstance(0);

  if (gji == nullptr)
  {
    return;
  }

  auto axisValues = mJoystickInfo.getAxisValues();
  auto axisLeftX = gji->getController()->getAxisId(SDL_CONTROLLER_AXIS_LEFTX);
  auto xl = axisValues[axisLeftX] / 32768.0f;
  auto hatValue = mJoystickInfo.getHatValues().at(0);
  auto dpadLeftPressed = hatValue & SDL_HAT_LEFT;
  auto dpadRightPressed = hatValue & SDL_HAT_RIGHT;
  if (dpadLeftPressed)
  {
     xl = -1.0f;
  }
  else if (dpadRightPressed)
  {
     xl = 1.0f;
  }

  if (fabs(xl)> 0.3f)
  {
     if (xl < 0.0f)
     {
       if (!isControllerActionSkipped())
       {
         mJoystickUpdateTime = GlobalClock::getInstance()->getElapsedTimeInS();
         left();
       }
     }
     else
     {
       if (!isControllerActionSkipped())
       {
         mJoystickUpdateTime = GlobalClock::getInstance()->getElapsedTimeInS();
         right();
       }
     }
  }
  else
  {
    mJoystickUpdateTime = 0.0f;
  }
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::update(float /*dt*/)
{
  mCursorPosition.x = dist * 0.5f + mSelectedItem * (quadWidth + dist) - 0.5f;
  updateControllerActions();
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::left()
{
  if (!mActive)
  {
    return;
  }

  if (mSelectedItem > 0)
  {
    mSelectedItem--;
  }
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::right()
{
  if (!mActive)
  {
    return;
  }

  if (mSelectedItem < getInventory()->size() - 1)
  {
    mSelectedItem++;
  }
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::show()
{
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::hide()
{
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::setActive(bool active)
{
  mActive = active;
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::confirm()
{
  if (!mActive)
  {
    return;
  }

  hide();
}


//---------------------------------------------------------------------------------------------------------------------
void InventoryLayer::cancel()
{
  if (!mActive)
  {
    return;
  }

  hide();
}

