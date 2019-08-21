#pragma once

#include <memory>
#include <string>
#include <vector>

#include "image/layer.h"


class MenuScreen
{

public:

   MenuScreen() = default;
   virtual ~MenuScreen() = default;

   virtual void update(const sf::Time& dt);
   void draw(sf::RenderTarget& window, sf::RenderStates states);

   const std::string& getFilename();
   void setFilename(const std::string& filename);

   void load();
   virtual void loadingFinished();

   virtual void keyboardKeyPressed(sf::Keyboard::Key key);
   virtual void keyboardKeyReleased(sf::Keyboard::Key key);

   bool isControllerUsed() const;


protected:

   std::string mFilename;
   std::vector<std::shared_ptr<Layer>> mLayerStack;
   std::map<std::string, std::shared_ptr<Layer>> mLayers;
};

