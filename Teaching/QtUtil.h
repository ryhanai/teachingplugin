#ifndef TEACHING_QT_UTIL_H_INCLUDED
#define TEACHING_QT_UTIL_H_INCLUDED

#ifdef _WIN32
#include <QtWidgets>
#else
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets> 
#endif
#endif

#endif
