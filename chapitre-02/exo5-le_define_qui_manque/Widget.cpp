#include "Widget.hpp"
#ifdef WITH_WIDGET
Widget::Widget() : m_value(42) {}
int Widget::GetValue() const { return m_value; }
#endif
