/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTINVOICES_H
#define POSTINVOICES_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_postInvoices.h"

class postInvoices : public XDialog, public Ui::postInvoices
{
    Q_OBJECT

public:
    postInvoices(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~postInvoices();

public slots:
    virtual void sPost();

protected slots:
    virtual void languageChange();

};

#endif // POSTINVOICES_H
