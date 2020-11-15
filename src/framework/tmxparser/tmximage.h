#pragma once

#include "tmxelement.h"

struct TmxImage : TmxElement
{
   TmxImage() = default;

   void deserialize(tinyxml2::XMLElement *e) override;

   std::string mSource;
   int mWidth = 0;
   int mHeight = 0;
};

