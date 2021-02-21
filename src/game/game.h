#pragma once

#include "console.h"
#include "constants.h"
#include "forestscene.h"
#include "infolayer.h"
#include "inventorylayer.h"
#include "overlays/controlleroverlay.h"
#include "overlays/rainoverlay.h"

#include "menus/menu.h"

#include "Box2D/Box2D.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <thread>
#include <future>


class Level;
class Player;


class Game
{
public:

   Game() = default;

   void initialize();
   int loop();
   void processEvents();
   void draw();


   void takeScreenshot();

private:

   void initializeWindow();
   void initializeController();

   void drawLevel();
   void loadLevel();
   void nextLevel();

   void reset();
   void resetAfterDeath(const sf::Time& dt);

   void update();
   void updateGameState(const sf::Time& dt);
   void updateGameController();
   void updateGameControllerForGame();
   void updateGameControllerForInventory();
   void updateAtmosphereShader();
   void updateWindowTitle();

   void checkCloseInventory();
   void openInventory();
   void showMainMenu();
   void showPauseMenu();

   void processKeyPressedEvents(const sf::Event& event);
   void processKeyReleasedEvents(const sf::Event& event);

   void toggleFullScreen();
   void togglePause();
   void changeResolution(int32_t w, int32_t h);


private:

   std::shared_ptr<Player> mPlayer;
   std::shared_ptr<sf::RenderWindow> mWindow;
   std::shared_ptr<sf::RenderTexture> mWindowRenderTexture;
   std::shared_ptr<Level> mLevel;
   std::unique_ptr<InfoLayer> mInfoLayer;
   std::unique_ptr<InventoryLayer> mInventoryLayer;
   std::unique_ptr<ControllerOverlay> mControllerOverlay;

   // temporary here for debugging only
   std::unique_ptr<ForestScene> mTestScene;
   bool mDrawTestScene = false;

   sf::Clock mDeltaClock;
   std::atomic<bool> mLevelLoadingFinished = false;
   std::atomic<bool> mLevelLoadingFinishedPrevious = false; // keep track of level loading in an async manner
   std::future<void> mLevelLoadingThread;
   bool mStoredPositionValid = false;
   sf::Vector2f mStoredPosition;

   int32_t mFps;
   bool mScreenshot = false;
   bool mDrawConsole = false;
   bool mDrawDebugInfo = false;
   bool mDrawControllerOverlay = false;
   bool mDrawCameraSystem = false;
   bool mDrawWeather = true;
   sf::Vector2u mRenderTextureOffset;
   int32_t mDeathWaitTimeMs = 0;

   bool mRecording = false;
   int32_t mRecordingCounter = 0;
   std::vector<sf::Image> mRecordingImages;
};

