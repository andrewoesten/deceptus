#include "gammashader.h"

#include "gameconfiguration.h"

#include <iostream>


//----------------------------------------------------------------------------------------------------------------------
void GammaShader::initialize(const sf::Texture& texture)
{
   if (!mGammaShader.loadFromFile("data/shaders/brightness.frag", sf::Shader::Fragment))
   {
      std::cout << "error loading gamma shader" << std::endl;
      return;
   }

   mGammaShader.setUniform("texture", texture);
}


//----------------------------------------------------------------------------------------------------------------------
void GammaShader::update()
{
   float gamma = 2.2f - (GameConfiguration::getInstance().mBrightness - 0.5f);
   mGammaShader.setUniform("gamma", gamma);
}


//----------------------------------------------------------------------------------------------------------------------
const sf::Shader& GammaShader::getGammaShader() const
{
   return mGammaShader;
}
