/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CURRENCIES_H
#define CURRENCIES_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_currencies.h"

class currencies : public XWidget, public Ui::currencies
{
    Q_OBJECT

public:
    currencies(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~currencies();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sSetCommentType();

protected slots:
    virtual void languageChange();

};

#endif // CURRENCIES_H
