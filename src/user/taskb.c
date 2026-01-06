void _start(int argc, char** argv) {
    while(1) {
        for(volatile int i=0; i < 10000000; i++);
    }
}