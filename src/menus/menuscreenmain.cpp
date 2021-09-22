#include "menuscreenmain.h"

#include "game/messagebox.h"
#include "game/gamestate.h"
#include "game/savestate.h"
#include "menu.h"


#define DEV_SAVE_STATE 1


MenuScreenMain::MenuScreenMain()
{
   setFilename("data/menus/titlescreen.psd");
}


void MenuScreenMain::update(const sf::Time& /*dt*/)
{

}


void MenuScreenMain::keyboardKeyPressed(sf::Keyboard::Key key)
{
   if (key == sf::Keyboard::Up)
   {
      up();
   }

   else if (key == sf::Keyboard::Down)
   {
      down();
   }

   else if (key == sf::Keyboard::Return)
   {
      select();
   }
}


void MenuScreenMain::loadingFinished()
{
   SaveState::deserializeFromFile();
   updateLayers();
}


void MenuScreenMain::up()
{
   switch (mSelection)
   {
      case Selection::Start:
         mSelection = Selection::Quit;
         break;
      case Selection::Options:
         mSelection = Selection::Start;
         break;
      case Selection::Quit:
         mSelection = Selection::Options;
         break;
   }

   updateLayers();
}


void MenuScreenMain::down()
{
   switch (mSelection)
   {
      case Selection::Start:
         mSelection = Selection::Options;
         break;
      case Selection::Options:
         mSelection = Selection::Quit;
         break;
      case Selection::Quit:
         mSelection = Selection::Start;
         break;
   }

   updateLayers();
}


void MenuScreenMain::select()
{
   switch (mSelection)
   {
      case Selection::Start:
#ifdef DEV_SAVE_STATE
      Menu::getInstance()->show(Menu::MenuType::FileSelect);
#else
      Menu::getInstance()->hide();
      GameState::getInstance().enqueueResume();
#endif
         break;
      case Selection::Options:
         Menu::getInstance()->show(Menu::MenuType::Options);
         break;
      case Selection::Quit:
         MessageBox::question(
            "Are you sure you want to quit?",
            [this](MessageBox::Button button) {if (button == MessageBox::Button::Yes) mExitCallback();}
         );
         break;
   }
}


void MenuScreenMain::setExitCallback(MenuScreenMain::ExitCallback callback)
{
    mExitCallback = callback;
}


void MenuScreenMain::updateLayers()
{
   const auto canContinue = !SaveState::allEmpty();

   mLayers["continue_0"]->_visible = canContinue && (mSelection != Selection::Start);
   mLayers["continue_1"]->_visible = canContinue && (mSelection == Selection::Start);

   mLayers["start_0"]->_visible = !canContinue && (mSelection != Selection::Start);
   mLayers["start_1"]->_visible = !canContinue && (mSelection == Selection::Start);

   mLayers["options_0"]->_visible = (mSelection != Selection::Options);
   mLayers["options_1"]->_visible = (mSelection == Selection::Options);

   mLayers["quit_0"]->_visible = (mSelection != Selection::Quit);
   mLayers["quit_1"]->_visible = (mSelection == Selection::Quit);
}



/*
data/menus/titlescreen.psd
    bg_temp
    quit_0
    quit_1
    options_0
    options_1
    start_0
    start_1
    version
    credits
    logo
*/
