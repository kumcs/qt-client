/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspBacklogByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <parameter.h>
#include <openreports.h>

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "printPackingList.h"

dspBacklogByItem::dspBacklogByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showPrices, SIGNAL(toggled(bool)), this, SLOT(sHandlePrices(bool)));
  connect(_soitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cSold);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->addColumn(tr("S/O #"),   _orderColumn, Qt::AlignLeft,  true, "cohead_number");
  _soitem->addColumn(tr("#"),         _seqColumn, Qt::AlignCenter,true, "coitem_linenumber");
  _soitem->addColumn(tr("Customer"),          -1, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  _soitem->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  _soitem->addColumn(tr("Qty. UOM"),  _qtyColumn, Qt::AlignRight, true, "uom_name");
  _soitem->addColumn(tr("Ordered"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Shipped"),   _qtyColumn, Qt::AlignRight, true, "coitem_qtyshipped");
  _soitem->addColumn(tr("Balance"),   _qtyColumn, Qt::AlignRight, true, "qtybalance");
  if (_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"))
    _soitem->addColumn(tr("Ext. Price"), _bigMoneyColumn, Qt::AlignRight, true, "baseextpricebalance");

  _showPrices->setEnabled(_privileges->check("ViewCustomerPrices") || _privileges->check("MaintainCustomerPrices"));

  if (! _showPrices->isChecked())
    _soitem->hideColumn("baseextpricebalance");

  _item->setFocus();
}

dspBacklogByItem::~dspBacklogByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBacklogByItem::languageChange()
{
  retranslateUi(this);
}

void dspBacklogByItem::sHandlePrices(bool pShowPrices)
{
  if (pShowPrices)
    _soitem->showColumn("baseextpricebalance");
  else
    _soitem->hideColumn("baseextpricebalance");

  sFillList();
}

bool dspBacklogByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Item Number"),
                         tr("<p>You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  _dates->appendValue(params);
  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("openOnly");
  params.append("orderByScheddate");

  if (_showPrices->isChecked())
    params.append("showPrices");

  return true;
}

void dspBacklogByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BacklogByItemNumber", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBacklogByItem::sEditOrder()
{
  salesOrder::editSalesOrder(_soitem->altId(), false);
}

void dspBacklogByItem::sViewOrder()
{
  salesOrder::viewSalesOrder(_soitem->altId());
}

void dspBacklogByItem::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("soitem_id", _soitem->id());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("soitem_id", _soitem->id());
      
  salesOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPrintPackingList()
{
  ParameterList params;
  params.append("sohead_id", _soitem->altId());

  printPackingList newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspBacklogByItem::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
  if (!_privileges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);


  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Item..."), this, SLOT(sEditItem()), 0);
  if (!_privileges->check("MaintainSalesOrders"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Item..."), this, SLOT(sViewItem()), 0);
  if ((!_privileges->check("MaintainSalesOrders")) && (!_privileges->check("ViewSalesOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print Packing List..."), this, SLOT(sPrintPackingList()), 0);
  if (!_privileges->check("PrintPackingLists"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspBacklogByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad(":/so/displays/SalesOrderItems.mql");
  q = mql.toQuery(params);
  _soitem->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
