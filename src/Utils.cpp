
#include "utils.hpp"

namespace tomato::util
{
#if 0
void
DeletionQueue::pushFunction(std::function<void()>&& voidFunc)
{
	deletors_.push_back(voidFunc);
}

void
DeletionQueue::flush()
{
	std::cout << "Flushing deletion queue...\n";

	for (auto rit = deletors_.rbegin(); rit != deletors_.rend(); ++rit) {
		(*rit)();
	}
	deletors_.clear();
}
#endif 
}  // namespace tomato::util
