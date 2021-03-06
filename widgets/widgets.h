/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef widgets_h
#define widgets_h

#define cNew                  1
#define cEdit                 2
#define cView                 3

#include <metrics.h>

#include "parameter.h"

#include <xtuplecommon.h>

#ifdef Q_OS_WIN
  #ifdef MAKEDLL
    #define XTUPLEWIDGETS_EXPORT __declspec(dllexport)
  #else
    #define XTUPLEWIDGETS_EXPORT
  #endif
#else
  #define XTUPLEWIDGETS_EXPORT
#endif

#ifndef Q_ENUM  /* introduced in Qt 5.5 */
#define Q_ENUM(ENUMERATION) Q_ENUMS(ENUMERATION)
#endif

class Metrics;
class Preferences;
class Privileges;
class QMdiArea;
class QScriptEngine;
class QWidget;
class GuiClientInterface;

extern Preferences *_x_preferences;
extern Metrics     *_x_metrics;
extern QMdiArea    *_x_workspace;
extern Privileges  *_x_privileges;
extern QString     _x_username;

void setupWidgetsScriptApi(QScriptEngine *engine, GuiClientInterface *client = 0);

#endif

