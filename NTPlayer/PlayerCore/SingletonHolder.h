#ifndef _PLAYERCORE_SINGLETONHOLDER_H_
#define _PLAYERCORE_SINGLETONHOLDER_H_

#include "Mutex.h"

template <class T>
class SingletonHolder
    /// This is a helper template class for managing
    /// singleton objects allocated on the heap.
    /// The class ensures proper deletion (including
    /// calling of the destructor) of singleton objects
    /// when the application that created them terminates.
{
public:
    SingletonHolder():
      instance_(0)
          /// Creates the SingletonHolder.
      {
      }

      ~SingletonHolder()
          /// Destroys the SingletonHolder and the singleton
          /// object that it holds.
      {
          delete instance_;
      }

      T* get()
          /// Returns a pointer to the singleton object
          /// hold by the SingletonHolder. The first call
          /// to get will create the singleton.
      {
          FastMutex::ScopedLock lock(mutex_);
          if (!instance_)
              instance_ = new T;
          return instance_;
      }

private:
    T* instance_;
    FastMutex mutex_;
};

#endif