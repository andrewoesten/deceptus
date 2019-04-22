// base
#include "luanode.h"

// lua
#include "lua/lua.hpp"

// stl
#include <sstream>
#include <thread>

// game
#include "audio.h"
#include "constants.h"
#include "level.h"
#include "fixturenode.h"
#include "player.h"
#include "luaconstants.h"
#include "luainterface.h"
#include "timer.h"

// static
std::atomic<int> LuaNode::sNextId = 0;

#define OBJINSTANCE LuaInterface::instance()->getObject(state)


extern "C" int updateProperties(lua_State* state)
{
   lua_pushnil(state);

   while(lua_next(state, -2) != 0)
   {
      std::string key = lua_tostring(state, -2);

      if (lua_isboolean(state, -1)) // bool
      {
         OBJINSTANCE->mProperties[key] = static_cast<bool>(lua_toboolean(state, -1));
         // printf("%s = %d\n", key.c_str(), lua_toboolean(state, -1));
      }
      if (lua_isnumber(state, -1))
      {
         if (lua_isinteger(state, -1)) // int64
         {
            OBJINSTANCE->mProperties[key] = static_cast<int64_t>(lua_tointeger(state, -1));
            // printf("%s = %lld\n", key.c_str(), lua_tointeger(state, -1));
         }
         else // double
         {
            OBJINSTANCE->mProperties[key] = lua_tonumber(state, -1);
            // printf("%s = %f\n", key.c_str(), lua_tonumber(state, -1));
         }
      }
      else if (lua_isstring(state, -1)) // string
      {
         OBJINSTANCE->mProperties[key] = std::string(lua_tostring(state, -1));
         // printf("%s = %s\n", key.c_str(), lua_tostring(state, -1));
      }

      // process nested tables
      //
      //      else if(lua_istable(state, -1))
      //      {
      //         return updateProperties(state);
      //      }

      lua_pop(state, 1);
   }

   OBJINSTANCE->synchronizeProperties();

   return 0;
}


extern "C" int updateSpriteRect(lua_State* state)
{
   // number of function arguments are on top of the stack.
   auto argc = lua_gettop(state);

   if (argc == 4)
   {
      auto x = static_cast<int32_t>(lua_tointeger(state, 1));
      auto y = static_cast<int32_t>(lua_tointeger(state, 2));
      auto w = static_cast<int32_t>(lua_tointeger(state, 3));
      auto h = static_cast<int32_t>(lua_tointeger(state, 4));

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->updateSpriteRect(x, y, w, h);
   }

   return 0;
}



extern "C" int damage(lua_State* state)
{
   // number of function arguments are on top of the stack.
   auto argc = lua_gettop(state);

   if (argc == 4)
   {
      auto playerId = static_cast<int32_t>(lua_tointeger(state, 1));
      auto damage = static_cast<int32_t>(lua_tonumber(state, 2));
      auto dx = static_cast<float>(lua_tonumber(state, 3));
      auto dy = static_cast<float>(lua_tonumber(state, 4));

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->damage(playerId, damage, dx, dy);
   }

   return 0;
}


extern "C" int boom(lua_State* state)
{
   // number of function arguments are on top of the stack.
   auto argc = lua_gettop(state);

   if (argc == 3)
   {
      auto x = static_cast<float>(lua_tonumber(state, 1));
      auto y = static_cast<float>(lua_tonumber(state, 2));
      auto intensity = static_cast<float>(lua_tonumber(state, 3));

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->boom(x, y, intensity);
   }

   return 0;
}


extern "C" int addShapeCircle(lua_State* state)
{
   auto argc = lua_gettop(state);

   if (argc == 3)
   {
      auto r = static_cast<float>(lua_tonumber(state, 1));
      auto x = static_cast<float>(lua_tonumber(state, 2));
      auto y = static_cast<float>(lua_tonumber(state, 3));

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->addShapeCircle(r, x, y);
   }

   return 0;
}


extern "C" int addShapeRect(lua_State* state)
{
   auto argc = lua_gettop(state);

   if (argc == 4)
   {
      auto width = static_cast<float>(lua_tonumber(state, 1));
      auto height = static_cast<float>(lua_tonumber(state, 2));
      auto x = static_cast<float>(lua_tonumber(state, 3));
      auto y = static_cast<float>(lua_tonumber(state, 4));

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->addShapeRect(width, height, x, y);
   }

   return 0;
}


extern "C" int addShapePoly(lua_State* state)
{
   auto argc = lua_gettop(state);

   if (argc >= 2 && (argc % 2 == 0))
   {
      auto size = argc / 2;
      b2Vec2* poly = new b2Vec2[size];
      auto polyIndex = 0;
      for (auto i = 0; i < argc; i += 2)
      {
         auto x = static_cast<float>(lua_tonumber(state, i));
         auto y = static_cast<float>(lua_tonumber(state, i + 1));
         poly[polyIndex].Set(x, y);
         polyIndex++;
      }

      std::shared_ptr<LuaNode> node = OBJINSTANCE;
      node->addShapePoly(poly, size);
   }

   return 0;
}


extern "C" int addWeapon(lua_State* state)
{
   auto argc = lua_gettop(state);
   auto fireInterval = 0;
   std::unique_ptr<b2Shape> shape;

   // add weapon with bullet radius only
   if (argc == 2)
   {
      fireInterval = static_cast<int>(lua_tointeger(state, 0));
      auto radius = static_cast<float>(lua_tonumber(state, 1));
      shape = std::make_unique<b2CircleShape>();
      dynamic_cast<b2CircleShape*>(shape.get())->m_radius = radius;
   }

   // add weapon with polygon bullet shape
   if (argc >= 3 && ((argc + 1) % 2 == 0))
   {
      fireInterval = static_cast<int>(lua_tointeger(state, 0));
      shape = std::make_unique<b2PolygonShape>();

      auto size = argc / 2;
      b2Vec2* poly = new b2Vec2[size];
      auto polyIndex = 0;
      for (auto i = 0; i < argc; i += 2)
      {
         auto x = static_cast<float>(lua_tonumber(state, i));
         auto y = static_cast<float>(lua_tonumber(state, i + 1));
         poly[polyIndex].Set(x, y);
         polyIndex++;
      }

      dynamic_cast<b2PolygonShape*>(shape.get())->Set(poly, polyIndex);
   }

   std::shared_ptr<LuaNode> node = OBJINSTANCE;
   node->addWeapon(std::move(shape), fireInterval);

   return 0;
}



extern "C" int timer(lua_State* state)
{
   // number of function arguments are on top of the stack.
   auto argc = lua_gettop(state);

   if (argc == 2)
   {
      auto delay = static_cast<int32_t>(lua_tointeger(state, 1));
      auto timerId = static_cast<int32_t>(lua_tointeger(state, 2));
      std::shared_ptr<LuaNode> node = OBJINSTANCE;

      Timer::add(
         std::chrono::milliseconds(delay),
         [node, timerId](){node->luaTimeout(timerId);}
      );

      //      std::thread([node, delay, timerId]() {
      //            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      //            node->luaTimeout(timerId);
      //         }
      //      ).detach();
   }

   return 0;
}


extern "C" int playSample(lua_State* state)
{
   // number of function arguments are on top of the stack.
   int argc = lua_gettop(state);

   if (argc == 2)
   {
      auto sampleId = static_cast<Audio::Sample>(lua_tointeger(state, 1));
      auto volume = static_cast<float>(lua_tonumber(state, 2));

      Audio::getInstance()->playSample(sampleId, volume);
   }

   return 0;
}


extern "C" int debug(lua_State* state)
{
   // number of function arguments are on top of the stack.
   int argc = lua_gettop(state);

   if (argc == 1)
   {
      const char* message = lua_tostring(state, 1);
      puts(message);
   }

   return 0;
}


void error(lua_State* state, const char* /*scope*/ = 0)
{
  // the error message is on top of the stack.
  // fetch it, print it and then pop it off the stack.
   std::stringstream os;
   os << lua_tostring(state, -1);

   printf("%s\n", os.str().c_str());

   lua_pop(state, 1);

   exit(1);
}


extern "C" int updateKeysPressed(lua_State* state)
{
   auto argc = lua_gettop(state);

   if (argc == 1)
   {
      auto keyPressed = static_cast<int32_t>(lua_tointeger(state, 1));

      auto obj = LuaInterface::instance()->getObject(state);
      if (obj != nullptr)
      {
         LuaInterface::instance()->updateKeysPressed(obj, keyPressed);
      }
   }

   return 0;
}


extern "C" int requestMap(lua_State* state)
{
   auto obj = LuaInterface::instance()->getObject(state);
   if (obj != nullptr)
   {
      LuaInterface::instance()->requestMap(obj);
   }

   return 0;
}


void LuaNode::setupTexture()
{
   std::string spriteName = std::get<std::string>(mProperties["sprite"]);
   if (mTexture.loadFromFile("data/" + spriteName))
   {
      // mSprite.scale(4.0f, 4.0f);
      mSprite.setTexture(mTexture);
   }
   else
   {
      printf("failed loading object spriteset");
   }
}


LuaNode::LuaNode(const std::string &filename)
 : GameNode(nullptr),
   mScriptName(filename)
{
}


void LuaNode::initialize()
{
   setupLua();
   createBody();
}


void LuaNode::setupLua()
{
   mState = luaL_newstate();

   // register callbacks
   lua_register(mState, "damage", ::damage);
   lua_register(mState, "debug", ::debug);
   lua_register(mState, "playSample", ::playSample);
   lua_register(mState, "updateProperties", ::updateProperties);
   lua_register(mState, "updateKeysPressed", ::updateKeysPressed);
   lua_register(mState, "timer", ::timer);
   lua_register(mState, "updateSpriteRect", ::updateSpriteRect);
   lua_register(mState, "addShapeCircle", ::addShapeCircle);
   lua_register(mState, "addShapeRect", ::addShapeRect);
   lua_register(mState, "boom", ::boom);

   // make standard libraries available in the Lua object
   luaL_openlibs(mState);

   // load program
   auto result = luaL_loadfile(mState, mScriptName.c_str());
   if (result == LUA_OK)
   {
      // execute program
      result = lua_pcall(mState, 0, LUA_MULTRET, 0);

      if (result != LUA_OK)
      {
         error(mState);
      }
      else
      {
         luaInitialize();
         luaRetrieveProperties();
         luaSendPatrolPath();
      }
   }
   else
   {
      error(mState);
   }
}


void LuaNode::synchronizeProperties()
{
   // evaluate property map
   //
   //   int i = std::get<int>(variant);
   //   w = std::get<int>(variant);
   //   w = std::get<0>(variant);

   // as soon as the texture is known, it can be set up
   setupTexture();
}


void LuaNode::luaMovedTo()
{
   float x = mPosition.x;
   float y = mPosition.y;

   lua_getglobal(mState, FUNCTION_MOVED_TO);

   lua_pushnumber(mState, x);
   lua_pushnumber(mState, y);

   // 3 args, 0 result
   auto result = lua_pcall(mState, 2, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_MOVED_TO);
   }
}


void LuaNode::luaPlayerMovedTo()
{
   sf::Vector2f pos =  Player::getPlayer(0)->getPixelPosition();

   lua_getglobal(mState, FUNCTION_PLAYER_MOVED_TO);

   lua_pushnumber(mState, pos.x);
   lua_pushnumber(mState, pos.y);

   // 3 args, 0 result
   auto result = lua_pcall(mState, 2, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_PLAYER_MOVED_TO);
   }
}


void LuaNode::luaRetrieveProperties()
{
   lua_getglobal(mState, FUNCTION_RETRIEVE_PROPERTIES);

   // 0 args, 0 result
   auto result = lua_pcall(mState, 0, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_RETRIEVE_PROPERTIES);
   }
}


void LuaNode::luaTimeout(int timerId)
{
   lua_getglobal(mState, FUNCTION_TIMEOUT);
   lua_pushinteger(mState, timerId);

   auto result = lua_pcall(mState, 1, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_TIMEOUT);
   }
}


void LuaNode::luaSendPath(const std::vector<sf::Vector2f>& vec)
{
   lua_newtable(mState);

   int i = 0;
   for (const auto& v : vec)
   {
      lua_pushnumber(mState, v.x); // push x
      lua_rawseti(mState,-2,++i);
      lua_pushnumber(mState, v.y); // push y
      lua_rawseti(mState,-2,++i);
   }
}


void LuaNode::damage(int playerId, int damage, float forceX, float forceY)
{
   Player::getPlayer(playerId)->damage(damage, sf::Vector2f(forceX, forceY));
}


void LuaNode::boom(float x, float y, float intensity)
{
   Level::getCurrentLevel()->boom(x, y, intensity);
}


void LuaNode::luaSendPatrolPath()
{
   if (mPatrolPath.size() == 0)
   {
      return;
   }

   lua_getglobal(mState, FUNCTION_SET_PATH);

   lua_pushstring(mState, "patrol_path");
   luaSendPath(mPatrolPath);

   // vec.size + 1 args, 0 result
   auto result = lua_pcall(mState, 2, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_SET_PATH);
   }
}



void LuaNode::luaDied()
{
}


bool LuaNode::getPropertyBool(const std::string& key)
{
   auto value = false;
   auto it = mProperties.find(key);
   if (it !=  mProperties.end())
      value = std::get<bool>(it->second);
   return value;
}


double LuaNode::getPropertyDouble(const std::string& key)
{
   auto value = 0.0;
   auto it = mProperties.find(key);
   if (it !=  mProperties.end())
      value = std::get<double>(it->second);
   return value;
}


int64_t LuaNode::getPropertyInt64(const std::string &key)
{
   auto value = 0LL;
   auto it = mProperties.find(key);
   if (it !=  mProperties.end())
      value = std::get<int64_t>(it->second);
   return value;
}


void LuaNode::createBody()
{
   auto staticBody = getPropertyBool("staticBody");
   auto damage = static_cast<int32_t>(getPropertyInt64("damage"));

   mBodyDef = std::make_shared<b2BodyDef>();
   mBodyDef->type = staticBody ? b2_staticBody : b2_dynamicBody;
   mBodyDef->position.Set(
      mStartPosition.x * MPP,
      mStartPosition.y * MPP
   );

   mBody = std::shared_ptr<b2Body>(
      Level::getCurrentLevel()->getWorld()->CreateBody(
         mBodyDef.get()),
         [](b2Body* body) {
            Level::getCurrentLevel()->getWorld()->DestroyBody(body);
         }
      );

   mBody->SetFixedRotation(true);

   for (auto shape : mShapes)
   {
      b2FixtureDef fd;
      fd.density = 1.f;
      fd.friction = 0.0f;
      fd.restitution = 0.0f;
      fd.shape = shape.get();

      // apply default filter
      // // http://www.iforce2d.net/b2dtut/collision-filtering
      fd.filter.groupIndex = mFilterDefaults.mGroupIndex;
      fd.filter.maskBits = mFilterDefaults.mMaskBits;
      fd.filter.categoryBits = mFilterDefaults.mCategoryBits;

      b2Fixture* ft = mBody->CreateFixture(&fd);
      FixtureNode* fn = new FixtureNode(this);
      fn->setType(ObjectTypeEnemy);
      fn->setProperty("damage", damage); // probably retrieve from player properties
      ft->SetUserData((void*)fn);
   }

   // if the startposition doesn't match the enemy boundaries in the future
   // the combined aabb will be required
   //
   //   // create aabb
   //   b2AABB aabb;
   //   aabb.lowerBound = b2Vec2(FLT_MAX,FLT_MAX);
   //   aabb.upperBound = b2Vec2(-FLT_MAX,-FLT_MAX);
   //   auto fixture = mBody->GetFixtureList();
   //   while (fixture != NULL)
   //   {
   //      aabb.Combine(aabb, fixture->GetAABB(0));
   //      fixture = fixture->GetNext();
   //   }
}


void LuaNode::addShapeCircle(float radius, float x, float y)
{
   std::shared_ptr<b2CircleShape> shape = std::make_shared<b2CircleShape>();
   shape->m_p.Set(x, y);
   shape->m_radius = radius;
   mShapes.push_back(shape);
}


void LuaNode::addShapeRect(float width, float height, float x, float y)
{
   std::shared_ptr<b2PolygonShape> shape = std::make_shared<b2PolygonShape>();
   shape->SetAsBox(width, height, b2Vec2(x, y), 0.0f);
   mShapes.push_back(shape);
}


void LuaNode::addShapePoly(const b2Vec2* points, int32_t size)
{
   std::shared_ptr<b2PolygonShape> shape = std::make_shared<b2PolygonShape>();
   shape->Set(points, size);
   mShapes.push_back(shape);
}


void LuaNode::addWeapon(std::unique_ptr<b2Shape> shape, int fireInterval)
{
   auto weapon = std::make_unique<Weapon>(std::move(shape), fireInterval);
   mWeapons.push_back(std::move(weapon));
}


void LuaNode::luaInitialize()
{
   lua_getglobal(mState, FUNCTION_INITIALIZE);
   auto result = lua_pcall(mState, 0, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_INITIALIZE);
   }
}


void LuaNode::luaAct(float dt)
{
   lua_getglobal(mState, FUNCTION_UPDATE);
   lua_pushnumber(mState, dt);

   auto result = lua_pcall(mState, 1, 0, 0);

   if (result != LUA_OK)
   {
      error(mState, FUNCTION_UPDATE);
   }
}


void LuaNode::stopScript()
{
   if (mState != nullptr)
   {
      lua_close(mState);
      mState = nullptr;

      printf("LuaInterface::StopScript: script stopped\n");
   }
}


void LuaNode::updateVelocity()
{
   auto velocityMax = 0.0;
   auto acceleration = 0.0;

   auto velIt = mProperties.find("velocity_walk_max");
   if (velIt != mProperties.end())
   {
      velocityMax = *std::get_if<double>(&(velIt->second));
   }

   auto accIt = mProperties.find("acceleration_ground");
   if (accIt != mProperties.end())
   {
      acceleration = *std::get_if<double>(&(accIt->second));
   }


   auto desiredVel = 0.0f;
   auto velocity = mBody->GetLinearVelocity();

   if (mKeysPressed & KeyPressedLeft)
   {
      desiredVel = static_cast<float>(b2Max(velocity.x - acceleration, -velocityMax));
   }

   if (mKeysPressed & KeyPressedRight)
   {
      desiredVel = static_cast<float>(b2Min( velocity.x + acceleration, velocityMax));
   }

   // calc impulse, disregard time factor
   auto velChange = desiredVel - velocity.x;
   auto impulse = mBody->GetMass() * velChange;

   mBody->ApplyLinearImpulse(
      b2Vec2(impulse, 0.0f),
      mBody->GetWorldCenter(),
      true
   );
}


void LuaNode::updatePosition()
{
   auto x = mBody->GetPosition().x * PPM;
   auto y = mBody->GetPosition().y * PPM;

   mPosition.x = x;
   mPosition.y = y;
}


void LuaNode::updateSpriteRect(int x, int y, int w, int h)
{
   mSpriteOffset.x = x;
   mSpriteOffset.y = y;
   mSpriteWidth = w;
   mSpriteHeight = h;
}


void LuaNode::draw(sf::RenderTarget &window)
{
   /*
   if (!mVisible)
   {
      return;
   }

   sf::Time time = mClock.getElapsedTime();

   if (time.asMilliseconds() >= mAnimSpeed)
   {
      mSpriteAnim.x++;

      if ((mSpriteAnim.x * SPRITE_WIDTH) >= mTexture.getSize().x)
         mSpriteAnim.x = 0;

      mClock.restart();
   }
   */

   mSprite.setTextureRect(
      sf::IntRect(
         mSpriteOffset.x,
         mSpriteOffset.y,
         mSpriteWidth,
         mSpriteHeight
      )
   );

   mSprite.setPosition(mPosition - sf::Vector2f(mSpriteWidth / 2.0f, mSpriteHeight / 2.0f));
   window.draw(mSprite);
}


