A very minimal User-mode OS.
Firstly, it initializes the memory(64MB RAM and 64MB disk), since this is a user mode OS, the memory is just a 128MB block of RAM using mmap
It then initalizes the memory manager, I have used implicit list technique to do this. Then the process table is initialized, with a maximum process count of 16

Now, the virtual file system(VFS) is initialized, utilizing the 64MB of RAM allocated. I used an implicit list to manage this memory aswell, as creating a proper
directory system felt too complicated for the very first project, and because I wanted to finish this project before my college semester started.
The VFS features its own custom functions that allows to perform various functions. The max files...(WIP)
