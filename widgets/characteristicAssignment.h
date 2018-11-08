/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHARACTERISTICASSIGNMENT_H
#define CHARACTERISTICASSIGNMENT_H

#include <parameter.h>

#include "ui_characteristicAssignment.h"

class CharacteristicAssignmentPrivate;

class characteristicAssignment : public QDialog, public Ui::characteristicAssignment
{
    Q_OBJECT

public:
    characteristicAssignment(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~characteristicAssignment();

public slots:
    virtual int  set(const ParameterList &pParams);
    virtual void sSave();
    virtual void sCheck();
    virtual void populate();
    virtual void sHandleChar();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _charassid;
    int _targetId;

    CharacteristicAssignmentPrivate *_d;
};

#endif // CHARACTERISTICASSIGNMENT_H
