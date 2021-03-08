#include <deque>
#include <string>
#include <vector>

class Console
{
   public:

      bool isActive() const;
      void setActive(bool active);

      void append(char c);
      void chop();
      void execute();
      void previousCommand();
      void nextCommand();

      const std::string& getCommand() const;
      const std::deque<std::string>& getLog() const;

      static Console& getInstance();


private:

      Console() = default;
      void showHelp();

      void giveWeaponBow();
      void giveWeaponDefault();

      bool mActive = false;
      std::string mCommand;

      std::vector<std::string> mHistory;
      int32_t mHistoryIndex = 0;

      std::deque<std::string> mLog;

      static Console mConsole;
};

