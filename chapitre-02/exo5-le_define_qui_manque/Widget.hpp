#pragma once
#ifdef WITH_WIDGET
class Widget {
public:
    Widget();
    int GetValue() const;
private:
    int m_value;
};
#else
class Widget {};
#endif
