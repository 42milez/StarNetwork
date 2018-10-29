#include "Singleton.h"

namespace base
{
  namespace
  {
    const size_t MAX_FINALIZERS_SIZE = 256;
    size_t num_finalizers = 0;
    base::SingletonFinalizer::FinalizerFunc finalizers[MAX_FINALIZERS_SIZE];
  }

  void base::SingletonFinalizer::add_finalizer(base::SingletonFinalizer::FinalizerFunc func)
  {
    assert(num_finalizers < MAX_FINALIZERS_SIZE);
    finalizers[num_finalizers++] = func;
  }

  void base::SingletonFinalizer::finalize()
  {
    for (auto i = num_finalizers - 1; i >= 0; --i) {
      (*finalizers[i])();
    }
    num_finalizers = 0;
  }
}
