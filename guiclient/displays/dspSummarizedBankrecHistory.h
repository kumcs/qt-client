/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSUMMARIZEDBANKRECHISTORY_H
#define DSPSUMMARIZEDBANKRECHISTORY_H

#include "display.h"

#include "ui_dspSummarizedBankrecHistory.h"

class dspSummarizedBankrecHistory : public display, public Ui::dspSummarizedBankrecHistory
{
    Q_OBJECT

public:
    dspSummarizedBankrecHistory(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

protected slots:
    virtual void languageChange();
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int);
    virtual void sReopen();

};

#endif // DSPSUMMARIZEDBANKRECHISTORY_H
