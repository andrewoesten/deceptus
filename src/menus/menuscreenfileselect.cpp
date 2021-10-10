#include "menuscreenfileselect.h"

#include "menu.h"

#include "game/gamestate.h"
#include "game/messagebox.h"
#include "game/savestate.h"

#include <iostream>
#include <ostream>
#include <sstream>


namespace {
   int32_t nameOffsetY = -2;
}


MenuScreenFileSelect::MenuScreenFileSelect()
{
   setFilename("data/menus/fileselect.psd");

   _font.loadFromFile("data/fonts/deceptum.ttf");
   const_cast<sf::Texture&>(_font.getTexture(12)).setSmooth(false);

   for (auto i = 0u; i < 3; i++)
   {
      _names[i].setFont(_font);
      _names[i].setCharacterSize(12);
      _names[i].setFillColor(sf::Color{232, 219, 243});
   }
}


void MenuScreenFileSelect::draw(sf::RenderTarget& window, sf::RenderStates states)
{
   MenuScreen::draw(window, states);

   for (auto i = 0u; i < 3; i++)
   {
      window.draw(_names[i], states);
   }
}


void MenuScreenFileSelect::up()
{
   auto idx = static_cast<int32_t>(_slot);
   idx--;
   if (idx < 0)
   {
      idx = 0;
   }
   _slot = static_cast<Slot>(idx);
   updateLayers();
}


void MenuScreenFileSelect::down()
{
   auto idx = static_cast<int32_t>(_slot);
   idx++;
   if (idx > 2)
   {
      idx = 2;
   }
   _slot = static_cast<Slot>(idx);
   updateLayers();
}


void MenuScreenFileSelect::select()
{
   SaveState::setCurrent(static_cast<uint32_t>(_slot));

   auto& saveState = SaveState::getCurrent();
   if (saveState.isEmpty())
   {
      // if current slot is empty, create a new slot and go to name select
      Menu::getInstance()->show(Menu::MenuType::NameSelect);
   }
   else
   {
      // if current slot holds data, load it
      Menu::getInstance()->hide();
      GameState::getInstance().enqueueResume();

      // request level-reloading since we updated the save state
      SaveState::getCurrent()._load_level_requested = true;
   }
}


void MenuScreenFileSelect::back()
{
   Menu::getInstance()->show(Menu::MenuType::Main);
}


void MenuScreenFileSelect::remove()
{
   MessageBox::question(
      "Are you sure you want to delete this file?",
      [this](MessageBox::Button button) {
         if (button == MessageBox::Button::Yes)
         {
            SaveState::getSaveState(static_cast<uint32_t>(_slot)).invalidate();
            SaveState::serializeToFile();
            updateLayers();
         }
      }
   );
}


void MenuScreenFileSelect::keyboardKeyPressed(sf::Keyboard::Key key)
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

   else if (key == sf::Keyboard::Escape)
   {
      back();
   }

   else if (key == sf::Keyboard::Delete)
   {
      remove();
   }
}


void MenuScreenFileSelect::controllerButtonX()
{
   remove();
}


void MenuScreenFileSelect::loadingFinished()
{
   SaveState::deserializeFromFile();
   updateLayers();
}


void MenuScreenFileSelect::showEvent()
{
   // always update to latest save state when the menu is shown.
   // when a new game is started, the save state changes, so the
   // layers become outdated.
   updateLayers();
}


void MenuScreenFileSelect::updateLayers()
{
   auto index = 0;

   const auto& saveStates = SaveState::getSaveStates();
   for (const auto& saveState : saveStates)
   {
      std::ostringstream out;
      out << (index + 1);

      const auto empty = saveState.isEmpty();
      const auto slotName = out.str();
      const auto selected = index == static_cast<int32_t>(_slot);

      // no data
      _layers["slot_" + slotName + "_new_game"]->_visible = empty;
      _layers["slot_" + slotName + "_new_game_background"]->_visible = empty;
      _layers["slot_" + slotName + "_new_game_highlight"]->_visible = empty;
      _layers["slot_" + slotName + "_new_game_deselected"]->_visible = empty && !selected;
      _layers["slot_" + slotName + "_new_game_selected"]->_visible = empty && selected;
      _layers["slot_" + slotName + "_shadow"]->_visible = empty;

      // have data
      _layers["slot_" + slotName + "_selected"]->_visible = !empty && selected;
      _layers["slot_" + slotName + "_deselected"]->_visible = !empty && !selected;
      _layers["slot_" + slotName + "_background"]->_visible = !empty;
      _layers["slot_" + slotName + "_bar_1"]->_visible = !empty;
      _layers["slot_" + slotName + "_bar_2"]->_visible = !empty;
      _layers["slot_" + slotName + "_energy"]->_visible = !empty;
      _layers["slot_" + slotName + "_heart"]->_visible = !empty;
      _layers["slot_" + slotName + "_highlight"]->_visible = !empty;
      _layers["slot_" + slotName + "_letter_deselected"]->_visible = !selected;
      _layers["slot_" + slotName + "_letter_selected"]->_visible = selected;
      _layers["slot_" + slotName + "_lines"]->_visible = !empty;
      _layers["slot_" + slotName + "_name"]->_visible = false;
      _layers["slot_" + slotName + "_progress"]->_visible = false;
      _layers["slot_" + slotName + "_time"]->_visible = false;

      // both
      _layers["slot_" + slotName + "_arrow"]->_visible = selected;

      // update names
      auto nameLayer = _layers["slot_" + slotName + "_name"];
      _names[index].setString(saveState._player_info.mName);
      _names[index].setPosition(
         nameLayer->_sprite->getPosition().x,
         nameLayer->_sprite->getPosition().y + nameOffsetY
      );

      index++;
   }

   _layers["delete_xbox_0"]->_visible = isControllerUsed();
   _layers["delete_xbox_1"]->_visible = false;
   _layers["delete_pc_0"]->_visible = !isControllerUsed();
   _layers["delete_pc_1"]->_visible = false;

   _layers["confirm_xbox_0"]->_visible = isControllerUsed();
   _layers["confirm_xbox_1"]->_visible = false;
   _layers["confirm_pc_0"]->_visible = !isControllerUsed();
   _layers["confirm_pc_1"]->_visible = false;

   _layers["back_xbox_0"]->_visible = isControllerUsed();
   _layers["back_xbox_1"]->_visible = false;
   _layers["back_pc_0"]->_visible = !isControllerUsed();
   _layers["back_pc_1"]->_visible = false;
}


