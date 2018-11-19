/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRICINGSCHEDULEASSIGNMENTS_H
#define PRICINGSCHEDULEASSIGNMENTS_H

#include "xwidget.h"
#include "ui_pricingScheduleAssignments.h"

class pricingScheduleAssignments : public XWidget, public Ui::pricingScheduleAssignments
{
    Q_OBJECT

public:
    pricingScheduleAssignments(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~pricingScheduleAssignments();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPrint();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    bool _listpricesched;
  
};

#endif // PRICINGSCHEDULEASSIGNMENTS_H
