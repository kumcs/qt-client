/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cmheadcluster.h"

CmheadCluster::CmheadCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new CmheadClusterLineEdit(this, pName));
}

CmheadClusterLineEdit::CmheadClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "cmhead", "cmhead_id", "cmhead_number", "cmhead_invcnumber", 0, 0, pName)
{
    setTitles(tr("Return"), tr("Returns"));
}
