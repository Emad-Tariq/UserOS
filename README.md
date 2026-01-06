A very minimal User-mode Kernel.
Firstly, it initializes the memory(64MB RAM and 64MB disk), since this is a user mode OS, the memory is just a 128MB block of RAM using mmap
It then initalizes the memory manager, I have used implicit list technique to do this. Then the process table is initialized, with a maximum process count of 16

Now, the virtual file system(VFS) is initialized, utilizing the 64MB of RAM allocated. I used an implicit list to manage this memory aswell, as creating a proper
directory system felt too complicated for the very first project, and because I wanted to finish this project before my college semester started.
The VFS features its own custom functions that allows to perform various functions. The max files is 32.

Finally, the timer interrupt is initialized(At the end, so it doesn't interrupt any other inits)
Now, the first user process is created, the Shell. The context is then switched to the shell, which functions similar to the linux Shell.

To execute a program, one must make the file in the "user" directory(obviously, glibc is not recognized). During bootup, the ELF file of user tasks is loaded
onto the vfs so that the ELF can be accessed by the loader, which loads the ELF using a naive implementation(directly loading all relevant segments).
To execute: "exec <name>.elf", the exec command calls the loader, which searches the VFS for <name>.elf, if found, it loads it onto the program's address space.
The address space is a 64KB space allocated using implicit list.

The processes are setup in a way that they free up the address space once terminated.

This is, by no means complete. It has many flaws and limitations that I intend to fix/improve in future iterations of this. What this was, was an incredible learning experience
