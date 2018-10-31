#ifndef LIFE_SINGLETON_H
#define LIFE_SINGLETON_H

#include <mutex>

namespace base
{
  class SingletonFinalizer {
  public:
    typedef void (*FinalizerFunc)();

    static void add_finalizer(FinalizerFunc func);

    static void finalize();
  };

  template<typename T>
  class Singleton final {
  public:
    static T &Instance() {
      std::call_once(init_flag, Init);
      assert(instance_);
      return *instance_;
    }

  private:
    static void Init() {
      instance_ = new T;
      SingletonFinalizer::add_finalizer(&Singleton<T>::Delete);
    }

    static void Delete() {
      delete instance_;
      instance_ = nullptr;
    }

    static std::once_flag init_flag;

    static T *instance_;
  };

  template<typename T>
  std::once_flag Singleton<T>::init_flag;

  template<typename T>
  T *Singleton<T>::instance_ = nullptr;
} // namespace base

#endif // LIFE_SINGLETON_H
