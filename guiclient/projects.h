/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PROJECTS_H
#define PROJECTS_H

#include "display.h"
#include "xtreewidget.h"
#include "ui_projects.h"

class projects : public display, public Ui::projects
{
    Q_OBJECT

public:
    projects(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sCopy();
    virtual void sOpen();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem*, int);
    virtual bool setParams(ParameterList &);
    virtual void sBuildList();	

private:
  QStringList _statuses;
  virtual void open(XTreeWidgetItem*, QString);
  virtual bool getPriv(int, XTreeWidgetItem*);
  
};

#endif // PROJECTS_H
