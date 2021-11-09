#include <vector>

namespace tomato
	{
    class TestClass
    {
    public:
      TestClass(const char* name)
      {
        name_ = name;
      }

      void doThing()
      {
        for (int i {}; i < 10000; ++i){
          intArr_.push_back(i);
        }
      }
    private:
      const char* name_;
      std::vector<int> intArr_;
    };
  }
