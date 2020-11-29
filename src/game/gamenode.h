#pragma once

#include <cstdint>
#include <string>
#include <vector>

class GameNode
{
public:

   GameNode(GameNode* parent = nullptr);
   virtual ~GameNode() = default;

   GameNode* getParent() const;
   void dump(int32_t depth = 0);

protected:
   void setName(const std::string& name);

private:
   std::string _name;
   std::vector<GameNode*> _children;
   GameNode* _parent = nullptr;
};

