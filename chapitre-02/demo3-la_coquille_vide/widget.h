#pragma once

class Widget {
public:
#ifdef WITH_EXTRA
    void configure(int a, int b);
#else
    void configure(int a);
#endif
};
