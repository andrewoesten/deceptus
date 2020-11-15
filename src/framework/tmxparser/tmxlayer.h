#pragma once

#include "tmxchunk.h"
#include "tmxelement.h"

struct TmxProperties;

struct TmxLayer : TmxElement
{
   TmxLayer();

   void deserialize(tinyxml2::XMLElement*) override;

   uint32_t mWidth = 0;
   uint32_t mHeight = 0;
   float mOpacity = 1.0f;
   bool mVisible = true;
   TmxProperties* mProperties = nullptr;
   int32_t mZ = 0;

   int32_t mOffsetX = 0;
   int32_t mOffsetY = 0;

   int32_t* mData = nullptr;

  std::vector<TmxChunk*> chunks;
};

