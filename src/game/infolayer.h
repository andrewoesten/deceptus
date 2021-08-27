#pragma once

#include "bitmapfont.h"

#include "framework/image/layer.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <memory>


class InfoLayer
{
public:

   InfoLayer();

   void draw(sf::RenderTarget& window, sf::RenderStates = sf::RenderStates::Default);
   void drawDebugInfo(sf::RenderTarget& window);
   void drawConsole(sf::RenderTarget& window, sf::RenderStates states = sf::RenderStates::Default);

   void setLoading(bool loading);

private:

   BitmapFont mFont;

   bool mLoading = false;
   sf::Time mShowTime;

   std::vector<std::shared_ptr<Layer>> mLayerStack;
   std::map<std::string, std::shared_ptr<Layer>> mLayers;
};

