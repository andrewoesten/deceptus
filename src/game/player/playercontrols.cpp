#include "playercontrols.h"

#include "constants.h"
#include "gamecontrollerintegration.h"
#include "framework/joystick/gamecontroller.h"


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::update(const sf::Time& /*dt*/)
{
   setWasMoving(isMovingHorizontally());
   setWasMovingLeft(isMovingLeft());
   setWasMovingRight(isMovingRight());
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::addKeypressedCallback(const KeypressedCallback& callback)
{
   _keypressed_callbacks.push_back(callback);
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::hasFlag(int32_t flag) const
{
   return _keys_pressed & flag;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::forceSync()
{
   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
   {
      _keys_pressed |= KeyPressedJump;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
   {
      _keys_pressed |= KeyPressedLook;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
   {
      _keys_pressed |= KeyPressedUp;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
   {
      _keys_pressed |= KeyPressedDown;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
   {
      _keys_pressed |= KeyPressedLeft;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
   {
      _keys_pressed |= KeyPressedRight;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
   {
      _keys_pressed |= KeyPressedRun;
   }

   if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
   {
      _keys_pressed |= KeyPressedFire;
   }
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::keyboardKeyPressed(sf::Keyboard::Key key)
{
   if (GameControllerIntegration::getInstance().isControllerConnected())
   {
      return;
   }

   if (key == sf::Keyboard::Space)
   {
      _keys_pressed |= KeyPressedJump;
   }
   else if (key == sf::Keyboard::LShift)
   {
      _keys_pressed |= KeyPressedLook;
   }
   else if (key == sf::Keyboard::Up)
   {
      _keys_pressed |= KeyPressedUp;
   }
   else if (key == sf::Keyboard::Down)
   {
      _keys_pressed |= KeyPressedDown;
   }
   else if (key == sf::Keyboard::Left)
   {
      _keys_pressed |= KeyPressedLeft;
   }
   else if (key == sf::Keyboard::Right)
   {
      _keys_pressed |= KeyPressedRight;
   }
   else if (key == sf::Keyboard::LAlt)
   {
      _keys_pressed |= KeyPressedRun;
   }
   else if (key == sf::Keyboard::LControl)
   {
      _keys_pressed |= KeyPressedFire;
   }

   for (auto& callback : _keypressed_callbacks)
   {
      callback(key);
   }
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::keyboardKeyReleased(sf::Keyboard::Key key)
{
   if (GameControllerIntegration::getInstance().isControllerConnected())
   {
      return;
   }

   if (key == sf::Keyboard::LShift)
   {
      _keys_pressed &= ~KeyPressedLook;
   }
   else if (key == sf::Keyboard::Up)
   {
      _keys_pressed &= ~KeyPressedUp;
   }
   else if (key == sf::Keyboard::Down)
   {
      _keys_pressed &= ~KeyPressedDown;
   }
   else if (key == sf::Keyboard::Left)
   {
      _keys_pressed &= ~KeyPressedLeft;
   }
   else if (key == sf::Keyboard::Right)
   {
      _keys_pressed &= ~KeyPressedRight;
   }
   else if (key == sf::Keyboard::Space)
   {
      _keys_pressed &= ~KeyPressedJump;
   }
   else if (key == sf::Keyboard::LAlt)
   {
      _keys_pressed &= ~KeyPressedRun;
   }
   else if (key == sf::Keyboard::LControl)
   {
      _keys_pressed &= ~KeyPressedFire;
   }
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isLookingAround() const
{
  if (_keys_pressed & KeyPressedLook)
  {
    return true;
  }

  if (isControllerUsed())
  {
    return isControllerButtonPressed(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  }

  return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isControllerUsed() const
{
  return !_joystick_info.getAxisValues().empty();
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isControllerButtonPressed(int button_enum) const
{
   if (!GameControllerIntegration::getInstance().isControllerConnected())
   {
      return false;
   }

   _joystick_info.getButtonValues();

   // does not need to be mapped
   // auto button_id = gji->getController()->getButtonId(static_cast<SDL_GameControllerButton>(button_enum));
   // pressed = (_joystick_info.getButtonValues()[static_cast<size_t>(button_id)]);

   const auto pressed = (_joystick_info.getButtonValues()[static_cast<size_t>(button_enum)]);
   return pressed;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isFireButtonPressed() const
{
  if (_keys_pressed & KeyPressedFire)
  {
    return true;
  }

  if (isControllerUsed())
  {
    return isControllerButtonPressed(SDL_CONTROLLER_BUTTON_X);
  }

  return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isJumpButtonPressed() const
{
  if (_keys_pressed & KeyPressedJump)
  {
    return true;
  }

  if (isControllerUsed())
  {
     return isControllerButtonPressed(SDL_CONTROLLER_BUTTON_A);
  }

  return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isUpButtonPressed() const
{
   if (_keys_pressed & KeyPressedUp)
   {
     return true;
   }

   if (isControllerUsed())
   {
      return isControllerButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_UP);
   }

   return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isDownButtonPressed() const
{
   if (_keys_pressed & KeyPressedDown)
   {
      return true;
   }

   if (isControllerUsed())
   {
      return isControllerButtonPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
   }

   return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isDroppingDown() const
{

   return isJumpButtonPressed() && isMovingDown();
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isMovingLeft() const
{
  if (isControllerUsed())
  {
     const auto& axisValues = _joystick_info.getAxisValues();

     const auto axis_left_x = GameControllerIntegration::getInstance().getController()->getAxisIndex(SDL_CONTROLLER_AXIS_LEFTX);
     auto xl = axisValues[static_cast<size_t>(axis_left_x)] / 32767.0f;
     const auto hat_value = _joystick_info.getHatValues().at(0);

     const auto dpad_left_pressed = hat_value & SDL_HAT_LEFT;
     const auto dpad_right_pressed = hat_value & SDL_HAT_RIGHT;

     if (dpad_left_pressed)
     {
        xl = -1.0f;
     }
     else if (dpad_right_pressed)
     {
        xl = 1.0f;
     }

     if (fabs(xl) >  0.3f)
     {
        if (xl < 0.0f)
        {
           return true;
        }
     }
  }
  else
  {
     if (_keys_pressed & KeyPressedLeft)
     {
        return true;
     }
  }

  return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isMovingDown() const
{
   if (isControllerUsed())
   {
      const auto& axis_values = _joystick_info.getAxisValues();
      const auto axis_left_y = GameControllerIntegration::getInstance().getController()->getAxisIndex(SDL_CONTROLLER_AXIS_LEFTY);
      auto y1 = axis_values[static_cast<size_t>(axis_left_y)] / 32767.0f;
      const auto hat_value = _joystick_info.getHatValues().at(0);
      const auto dpad_down_pressed = hat_value & SDL_HAT_DOWN;
      const auto dpad_right_pressed = hat_value & SDL_HAT_UP;

      if (dpad_down_pressed)
      {
         y1 = 1.0f;
      }
      else if (dpad_right_pressed)
      {
         y1 = 1.0f;
      }

      if (fabs(y1) > 0.3f)
      {
         if (y1 > 0.0f)
         {
            return true;
         }
      }
   }
   else
   {
      if (_keys_pressed & KeyPressedDown)
      {
         return true;
      }
   }

   return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isMovingRight() const
{
   if (isControllerUsed())
   {
      const auto& axis_values = _joystick_info.getAxisValues();
      const auto axis_left_x = GameControllerIntegration::getInstance().getController()->getAxisIndex(SDL_CONTROLLER_AXIS_LEFTX);
      auto xl = axis_values[static_cast<size_t>(axis_left_x)] / 32767.0f;
      const auto hat_value = _joystick_info.getHatValues().at(0);
      const auto dpad_left_pressed = hat_value & SDL_HAT_LEFT;
      const auto dpad_right_pressed = hat_value & SDL_HAT_RIGHT;

      if (dpad_left_pressed)
      {
         xl = -1.0f;
      }
      else if (dpad_right_pressed)
      {
         xl = 1.0f;
      }

      if (fabs(xl) > 0.3f)
      {
         if (xl > 0.0f)
         {
            return true;
         }
      }
   }
   else
   {
      if (_keys_pressed & KeyPressedRight)
      {
         return true;
      }
   }

   return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::isMovingHorizontally() const
{
   return isMovingLeft() || isMovingRight();
}


//----------------------------------------------------------------------------------------------------------------------
int PlayerControls::getKeysPressed() const
{
   return _keys_pressed;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::setKeysPressed(int keysPressed)
{
   _keys_pressed = keysPressed;
}


//----------------------------------------------------------------------------------------------------------------------
const GameControllerInfo& PlayerControls::getJoystickInfo() const
{
   return _joystick_info;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::setJoystickInfo(const GameControllerInfo& joystick_info)
{
   _joystick_info = joystick_info;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::wasMoving() const
{
   return _was_moving;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::setWasMoving(bool was_moving)
{
   _was_moving = was_moving;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::wasMovingLeft() const
{
   return _was_moving_left;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::setWasMovingLeft(bool was_moving_left)
{
   _was_moving_left = was_moving_left;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::wasMovingRight() const
{
   return _was_moving_right;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerControls::setWasMovingRight(bool was_moving_right)
{
   _was_moving_right = was_moving_right;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::changedToIdle() const
{
   return wasMoving() && !isMovingHorizontally();
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerControls::changedToMoving() const
{
   return !wasMoving() && isMovingHorizontally();
}
