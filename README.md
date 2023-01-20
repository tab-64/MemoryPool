# MemoryPool

A header-only memory pool library implemented in C++.
一个只有头文件的，C++的内存池库

There are a lot of problems in it, but they will be solved at some point in the future.
这里面有很多问题，但会在未来被解决 

It's just for learning now, and it may be unsuitable for some conditions.
现在只是用来学习，并且它并不适用于某且情况 

If you found some problems or have some recommendations, welcome to make an 'Issue'.
如果你发现了一些问题或者有一些建议，欢迎开issue 

# Known problems 已知问题
- Won't check whether the memory address given to 'ObjectPool::release()' is released;
  不会检查提供给'ObjectPool::release()'的内存是否已经被释放
- ...

# TODO
- Solve the problems above
  解决上述问题
- Implement another version, which supports allocation of different size of memory.
  实现支持分配不同大小内存的版本
- ...
