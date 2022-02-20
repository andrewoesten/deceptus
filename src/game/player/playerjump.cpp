#include "playerjump.h"

#include "animationpool.h"
#include "audio.h"
#include "camerapanorama.h"
#include "framework/tools/globalclock.h"
#include "framework/tools/stopwatch.h"
#include "gamecontactlistener.h"
#include "physics/physicsconfiguration.h"
#include "savestate.h"

#include <Box2D/Box2D.h>
#include <iostream>


namespace
{
constexpr auto minimum_jump_interval_ms = 150;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::update(const PlayerJumpInfo& info)
{
   const auto was_in_air = _jump_info._in_air;

   _jump_info = info;

#ifdef JUMP_GRAVITY_SCALING
   if (was_in_air && !_jump_info._in_air)
   {
      // player touched ground
      _body->SetGravityScale(1.0f);
   }

   if (_jump_info._in_water)
   {
      _body->SetGravityScale(0.5f);
   }
#endif

   if (!_jump_info._in_air)
   {
      _double_jump_consumed = false;
   }

   updateLostGroundContact();
   updateJump();
   updateJumpBuffer();
   updateWallSlide();
   updateWallJump();
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::updateJumpBuffer()
{
   if (_jump_info._in_air)
   {
      return;
   }

   // if jump is pressed while the ground is just a few centimeters away,
   // store the information and jump as soon as the places touches ground
   auto now = GlobalClock::getInstance().getElapsedTime();
   auto time_diff = (now - _last_jump_press_time).asMilliseconds();

   if (time_diff < PhysicsConfiguration::getInstance()._player_jump_buffer_ms)
   {
      jump();
   }
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::updateJump()
{
   if (_jump_info._in_water && _controls->isJumpButtonPressed())
   {
      // only allow jumping out of the water / water movement if the player stayed inside the water for a bit
      using namespace std::chrono_literals;
      if ((StopWatch::getInstance().now() - _jump_info._water_entered_timepoint) > std::chrono::milliseconds(PhysicsConfiguration::getInstance()._player_in_water_time_to_allow_jump_button_ms))
      {
         _body->ApplyForce(
            b2Vec2{0, PhysicsConfiguration::getInstance()._player_in_water_force_jump_button},
            _body->GetWorldCenter(),
            true
         );
      }

      // std::cout << (StopWatch::getInstance().now() - _jump_info._water_entered_timepoint).count() << std::endl;
   }
   else if (
         (_jump_frame_count > 0 && _controls->isJumpButtonPressed())
      || _jump_clock.getElapsedTime().asMilliseconds() < PhysicsConfiguration::getInstance()._player_jump_minimal_duration_ms
   )
   {
      // jump higher if faster than regular walk speed
      auto max_walk = PhysicsConfiguration::getInstance()._player_speed_max_walk;
      auto vel = fabs(_body->GetLinearVelocity().x) - max_walk;
      auto factor = 1.0f;

      if (vel > 0.0f)
      {
         // probably dead code
         auto max_run = PhysicsConfiguration::getInstance()._player_speed_max_run;
         factor = 1.0f + PhysicsConfiguration::getInstance()._player_jump_speed_factor * (vel / (max_run - max_walk));
      }

      /*
       * +---+
         |###|
         |###| <- current speed => factor
         |###|
         +###+
         |   |
         |   |
         |   |
       * +---+
      */

      // to change velocity by 5 in one time step
      constexpr auto fixed_timestep = (1.0f / 60.0f);

      // f = mv / t
      auto force = factor * _body->GetMass() * PhysicsConfiguration::getInstance()._player_jump_strength / fixed_timestep;

      // spread the force over 6.5 time steps
      force /= PhysicsConfiguration::getInstance()._player_jump_falloff;

      // more force is required to compensate falling velocity for scenarios
      // - wall jump
      // - double jump
      if (_compensate_velocity)
      {
         const auto bodyVelocity = _body->GetLinearVelocity();
         force *= 1.75f * bodyVelocity.y;

         _compensate_velocity = false;
      }

      _body->ApplyForceToCenter(b2Vec2(0.0f, -force), true);

      _jump_frame_count--;

      if (_jump_frame_count == 0)
      {
         // out of 'push upward'-frames, now it goes down quickly until player hits ground
         _body->SetGravityScale(1.35f);
      }
   }
   else
   {
      _jump_frame_count = 0;
   }
}


//----------------------------------------------------------------------------------------------------------------------
// not used by the game
void PlayerJump::jumpImpulse()
{
   _jump_clock.restart();

   float impulse = _body->GetMass() * 6.0f;

   _body->ApplyLinearImpulse(
      b2Vec2(0.0f, -impulse),
      _body->GetWorldCenter(),
      true
   );
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::jumpImpulse(const b2Vec2& impulse)
{
   _jump_clock.restart();

   _body->ApplyLinearImpulse(
      impulse,
      _body->GetWorldCenter(),
      true
   );
}


//----------------------------------------------------------------------------------------------------------------------
// apply individual forces for a given number of frames
// that's the approach this game is currently using
void PlayerJump::jumpForce()
{
   _jump_clock.restart();
   _jump_frame_count = PhysicsConfiguration::getInstance()._player_jump_frame_count;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::doubleJump()
{
   if (_walljump_frame_count > 0)
   {
      return;
   }

   const auto skills = SaveState::getPlayerInfo()._extra_table._skills._skills;
   const auto can_double_jump = (skills & static_cast<int32_t>(ExtraSkill::Skill::DoubleJump));

   if (!can_double_jump)
   {
      return;
   }

   if (_double_jump_consumed)
   {
      return;
   }

   _double_jump_consumed = true;
   _compensate_velocity = true;

   // double jump should happen with a constant impulse, no adjusting through button press duration
   const auto current_velocity = _body->GetLinearVelocity();
   _body->SetLinearVelocity(b2Vec2(current_velocity.x, 0.0f));
   jumpImpulse(b2Vec2(0.0f, _body->GetMass() * PhysicsConfiguration::getInstance()._player_double_jump_factor));

   _timepoint_doublejump = StopWatch::now();
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::wallJump()
{
   const auto skills = SaveState::getPlayerInfo()._extra_table._skills._skills;
   const auto canWallJump = (skills & static_cast<int32_t>(ExtraSkill::Skill::WallJump));

   if (!canWallJump)
   {
      return;
   }

   if (!_wallsliding)
   {
      return;
   }

   _walljump_points_right = (GameContactListener::getInstance().getPlayerArmLeftContactCount() > 0);

   // double jump should happen with a constant impulse, no adjusting through button press duration
   _body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

   const auto impulse_x =   _body->GetMass() * PhysicsConfiguration::getInstance()._player_wall_jump_vector_x;
   const auto impulse_y = -(_body->GetMass() * PhysicsConfiguration::getInstance()._player_wall_jump_vector_y);

   _walljump_frame_count = PhysicsConfiguration::getInstance()._player_wall_jump_frame_count;
   _walljump_multiplier = PhysicsConfiguration::getInstance()._player_wall_jump_multiplier;
   _walljump_direction = b2Vec2(_walljump_points_right ? impulse_x : -impulse_x, impulse_y);
   _timepoint_walljump = StopWatch::now();
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::jump()
{
   if (_controls->isDownButtonPressed())
   {
      // equivalent to check for bending down, but jump can be called via lamda
      // which mich skip the bend state update in the player update loop.
      return;
   }

   if (CameraPanorama::getInstance().isLookActive())
   {
      return;
   }

   sf::Time elapsed = _jump_clock.getElapsedTime();

   // only allow a new jump after a a couple of milliseconds
   if (elapsed.asMilliseconds() > minimum_jump_interval_ms)
   {
      // handle regular jump
      if (!_jump_info._in_air || _ground_contact_just_lost || _jump_info._climbing)
      {
         _remove_climb_joint_callback();
         _jump_info._climbing = false; // only set for correctness until next frame

         jumpForce();

         if (_jump_info._in_water)
         {
            // play some waterish sample?
         }
         else
         {
            _dust_animation_callback();
            Audio::getInstance().playSample("jump.wav");
         }
      }
      else
      {
         // player pressed jump but is still in air.
         // buffer that information to trigger the jump a few millis later.
         if (_jump_info._in_air)
         {
            _last_jump_press_time = GlobalClock::getInstance().getElapsedTime();

            // handle wall jump
            wallJump();

            // handle double jump
            doubleJump();
         }
      }
   }
}



//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::updateLostGroundContact()
{
   // when losing contact to the ground allow jumping for 2-3 more frames
   //
   // if player had ground contact in previous frame but now lost ground
   // contact then start counting to 200ms
   if (_had_ground_contact && _jump_info._in_air && !isJumping())
   {
      auto now = GlobalClock::getInstance().getElapsedTime();
      _ground_contact_lost_time = now;
      _ground_contact_just_lost = true;
   }

   // flying now, probably allow jump
   else if (_jump_info._in_air)
   {
      auto now = GlobalClock::getInstance().getElapsedTime();
      auto timeDiff = (now - _ground_contact_lost_time).asMilliseconds();
      _ground_contact_just_lost = (timeDiff < PhysicsConfiguration::getInstance()._player_jump_after_contact_lost_ms);

      // if (mGroundContactJustLost)
      // {
      //    Log::Info() << "allowed to jump for another " << timeDiff << "ms";
      // }
   }
   else
   {
      _ground_contact_just_lost = false;
   }

   _had_ground_contact = !_jump_info._in_air;
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::updateWallSlide()
{
   if (!_jump_info._in_air)
   {
      _wallsliding = false;
      return;
   }

   // early out if walljump still active
   if (_walljump_frame_count > 0)
   {
      return;
   }

   const auto skills = SaveState::getPlayerInfo()._extra_table._skills._skills;
   const auto canWallSlide = (skills & static_cast<int32_t>(ExtraSkill::Skill::WallSlide));

   if (!canWallSlide)
   {
      _wallsliding = false;
      return;
   }

   const auto leftTouching = (GameContactListener::getInstance().getPlayerArmLeftContactCount() > 0);
   const auto rightTouching = (GameContactListener::getInstance().getPlayerArmRightContactCount() > 0);

   if (
         !(leftTouching  && _controls->isMovingLeft())
      && !(rightTouching && _controls->isMovingRight())
   )
   {
      _wallsliding = false;
      return;
   }

   b2Vec2 vel = _body->GetLinearVelocity();
   _body->ApplyForce(PhysicsConfiguration::getInstance()._player_wall_slide_friction * -vel, _body->GetWorldCenter(), false);

   if (!_wallsliding)
   {
      _timepoint_wallslide = StopWatch::now();
      _wallsliding = true;
   }
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::updateWallJump()
{
   if (_walljump_frame_count == 0)
   {
      return;
   }

   _walljump_multiplier *= PhysicsConfiguration::getInstance()._player_wall_jump_multiplier_scale_per_frame;
   _walljump_multiplier += PhysicsConfiguration::getInstance()._player_wall_jump_multiplier_increment_per_frame;

   _body->ApplyForceToCenter(_walljump_multiplier * _walljump_direction, true);

   // Log::Info() << "step: " << _walljump_steps << " " << _walljump_direction.x << " " << _walljump_direction.y;

   _walljump_frame_count--;
}


//----------------------------------------------------------------------------------------------------------------------
bool PlayerJump::isJumping() const
{
   return (_jump_frame_count > 0);
}


//----------------------------------------------------------------------------------------------------------------------
void PlayerJump::setControls(const std::shared_ptr<PlayerControls>& newControls)
{
   _controls = newControls;
}
