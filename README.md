# Dangling-pointer-killer
Access a pointer with safe RAII and not a smart pointer.

A ref will take a pointer that will point to a heap pointer which can access the real memory(pointer this).
And important, this is thread-safe if your real memory(poniter this) and yoru ref are involved in multi-thread.

Tip:
1. Ref::get() is just used to check valid with a short lock.
2. RefTaker::get() is used to normal logic but it will lock longer.
3. If you don't need a ref point to real memory any more, just call deref().
4. Translate Chinese comment yourself.
5. Do not put ref in vector that will cause balabala...you debug yourself.
6. Feed me back any bug in Issues.
