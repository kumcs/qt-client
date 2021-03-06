/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: rename unpostedReceipts
#include "unpostedPoReceipts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "enterPoReceipt.h"
#include "enterPoitemReceipt.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "mqlutil.h"
#include "purchaseOrderItem.h"
#include "storedProcErrorLookup.h"
#include "transferOrderItem.h"
#include "returnAuthorizationItem.h"
#include "errorReporter.h"

unpostedPoReceipts::unpostedPoReceipts(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,	          SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_post,          SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print,         SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_recv, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_viewOrderItem,    SIGNAL(clicked()), this, SLOT(sViewOrderItem()));
  connect(omfgThis, SIGNAL(purchaseOrderReceiptsUpdated()), this, SLOT(sFillList()));

  _recv->addColumn(tr("Order #"),       _orderColumn, Qt::AlignRight,  true, "recv_order_number"  );
  _recv->addColumn(tr("Type"),          50,           Qt::AlignCenter, true, "recv_order_type" );
  _recv->addColumn(tr("From"),          -1,           Qt::AlignLeft,   true, "orderhead_from"   );
  _recv->addColumn(tr("Line #"),        50,           Qt::AlignRight,  true, "orderitem_linenumber");
  _recv->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true, "recv_duedate");
  _recv->addColumn(tr("Site"),          _whsColumn,   Qt::AlignRight,  true, "warehous_code"  );
  _recv->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignRight,  true, "item_number");
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true, "uom_name");
  _recv->addColumn(tr("Vend. Item #"),  _itemColumn,  Qt::AlignLeft,   true, "recv_vend_item_number");
  _recv->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true, "recv_vend_uom");
  _recv->addColumn(tr("Ordered"),       _qtyColumn,   Qt::AlignRight,  true, "orderitem_qty_ordered");
  _recv->addColumn(tr("Received"),      _qtyColumn,   Qt::AlignRight,  true, "orderitem_qty_received");
  _recv->addColumn(tr("To Receive"),    _qtyColumn,   Qt::AlignRight,  true, "recv_qty");
  _recv->addColumn(tr("Receipt Date"),  _dateColumn,  Qt::AlignCenter, true, "recv_date");
  _recv->addColumn(tr("G/L Post Date"), _dateColumn,  Qt::AlignCenter, true, "recv_gldistdate");

  if (! _privileges->check("ChangePORecvPostDate"))
    _recv->hideColumn(_recv->column("recv_gldistdate"));

  if(!_privileges->check("ViewPurchaseOrders"))
    disconnect(_recv, SIGNAL(valid(bool)), _viewOrderItem, SLOT(setEnabled(bool)));

  sFillList();
}

unpostedPoReceipts::~unpostedPoReceipts()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedPoReceipts::languageChange()
{
  retranslateUi(this);
}

void unpostedPoReceipts::setParams(ParameterList & params)
{
  params.append("nonInventory",	tr("Non-Inventory"));
  params.append("na",		tr("N/A"));
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");
}

void unpostedPoReceipts::sPrint()
{
  ParameterList params;
  setParams(params);
  orReport report("UnpostedPoReceipts", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedPoReceipts::sNew()
{
  ParameterList params;

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPoReceipts::sEdit()
{
  ParameterList params;
  params.append("mode",		"edit");
  params.append("recv_id",	_recv->id());

  enterPoitemReceipt *newdlg = new enterPoitemReceipt();
  newdlg->set(params);
  newdlg->exec();
}

void unpostedPoReceipts::sDelete()
{
  XSqlQuery unpostedDelete;
  if (QMessageBox::question(this, tr("Cancel Receipts?"),
			    tr("<p>Are you sure you want to delete these "
			       "unposted Receipts?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    unpostedDelete.prepare( "DELETE FROM recv "
	       "WHERE (recv_id IN (:id));" );
    QList<XTreeWidgetItem*>selected = _recv->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      unpostedDelete.bindValue(":id", ((XTreeWidgetItem*)(selected[i]))->id() );
      unpostedDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Receipt Information"),
                                    unpostedDelete, __FILE__, __LINE__))
      {
        return;
      }
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
}

void unpostedPoReceipts::sViewOrderItem()
{
  ParameterList params;
  params.append("mode",		"view");

  QString ordertype = _recv->currentItem()->text(_recv->column("recv_order_type"));
  if (ordertype == "PO")
  {
    params.append("poitem_id",	_recv->altId());
    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (ordertype == "TO")
  {
    params.append("toitem_id",	_recv->altId());
    transferOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (ordertype == "RA")
  {
    params.append("raitem_id",	_recv->altId());
    returnAuthorizationItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void unpostedPoReceipts::sPost()
{
  XSqlQuery unpostedPost;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  bool tryagain = false;
  int succeeded = 0;
  QList<int> failedReceipts;
  QList<QString> failedItems;
  QList<QString> errors;
  do {
    bool changeDate = false;
    QDate newDate = QDate::currentDate();

    if (_privileges->check("ChangePORecvPostDate"))
    {
      getGLDistDate newdlg(this, "", true);
      newdlg.sSetDefaultLit(tr("Receipt Date"));
      if (newdlg.exec() == XDialog::Accepted)
      {
        newDate = newdlg.date();
        changeDate = (newDate.isValid());
      }
      else
        return;
    }

    XSqlQuery setDate;
    setDate.prepare("UPDATE recv SET recv_gldistdate=:distdate "
                   "WHERE recv_id=:recv_id;");

    QList<XTreeWidgetItem*>selected = _recv->selectedItems();
    QList<XTreeWidgetItem*>triedToClosed;

    // Update dates if user changed the transaction date after clicking post
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();
      if (changeDate)
      {
        setDate.bindValue(":distdate",  newDate);
        setDate.bindValue(":recv_id", id);
        setDate.exec();
        if (setDate.lastError().type() != QSqlError::NoError)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Receipt Information"),
                               setDate, __FILE__, __LINE__);
          return;
        }
      }
    }
    // Cycle through the selected items on the _recv list
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();
      int itemlocSeries = -1;

      XSqlQuery recvInfo;
      recvInfo.prepare("SELECT COALESCE(item_number, recv_vend_item_number) AS item_number, "
                      "  recv_id, itemsite_id, itemsite_controlmethod, itemsite_perishable, itemsite_warrpurc, "
                      "  (recv_order_type = 'RA' AND COALESCE(itemsite_costmethod,'') = 'J') AS issuewo, "
                      "  (recv_order_type = 'PO' AND COALESCE(itemsite_costmethod,'') = 'J' AND poitem_order_type='W') AS issuejobwo, "
                      "  (recv_order_type = 'PO' AND COALESCE(itemsite_costmethod,'') = 'J' AND poitem_order_type='S') AS issuejobso, "
                      "  COALESCE(pohead_dropship, false) AS dropship, recv_order_type, recv_order_number, "
                      "  roundQty(item_fractional, (recv_qty * orderitem_qty_invuomratio)) AS qty, "
                      "  isControlledItemsite(itemsite_id) AS controlled, "
                      "  recv_orderitem_id, recv_qty "
                      " FROM orderitem, recv "
                      "  LEFT OUTER JOIN itemsite ON (recv_itemsite_id=itemsite_id) "
                      "  LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
                      "  LEFT OUTER JOIN poitem ON ((recv_order_type='PO') "
                      "                         AND (recv_orderitem_id=poitem_id)) "
                      "  LEFT OUTER JOIN pohead ON (poitem_pohead_id=pohead_id) "
                      " WHERE orderitem_id = recv_orderitem_id "
                      "  AND orderitem_orderhead_type = recv_order_type "
                      "  AND recv_id = :recv_id "
                      " ORDER BY orderitem_linenumber;");
      recvInfo.bindValue(":recv_id", id);
      recvInfo.exec();
      if (!recvInfo.first())
      {
        failedReceipts.append(id);
        failedItems.append("NULL");
        errors.append(tr("Failed to retrieve recv and orderitem info. %1")
          .arg(recvInfo.lastError().text()));
        continue;
      }

      // Check if the date is in a closed period
      XSqlQuery closedPeriod;
      closedPeriod.prepare("SELECT period_closed "
                           "FROM recv "
                           "  JOIN period ON COALESCE(recv_gldistdate, recv_date) BETWEEN period_start AND period_end "
                           "WHERE recv_id=:recv_id;");
      closedPeriod.bindValue(":recv_id", id);
      closedPeriod.exec();
      if (!closedPeriod.first() || closedPeriod.value("period_closed").toBool())
      {
        failedReceipts.append(id);
        if (_privileges->check("ChangePORecvPostDate"))
        {
          if (changeDate)
          {
            triedToClosed = selected;
            break;
          }
          else
            triedToClosed.append(selected[i]);
        }
        continue;  
      }
      
      // Stage cleanup function to be called on error
      XSqlQuery cleanup;
      cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");

      // Get parent series id
      XSqlQuery parentSeries;
      parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
      parentSeries.exec();
      if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
      {
        itemlocSeries = parentSeries.value("result").toInt();
        cleanup.bindValue(":itemlocSeries", itemlocSeries);
      }
      else
      {
        failedReceipts.append(id);
        failedItems.append(recvInfo.value("item_number").toString());
        errors.append(tr("Failed to Retrieve the Next itemloc_series_seq. %1")
          .arg(parentSeries.lastError().text()));
        continue;
      }

      // itemlocdistId used for "to" wh side of transaction if Transfer Order & MultiWhs 
      int itemlocdistId = -1;
      XSqlQuery parentItemlocdist;    
   
      // If Transfer Order, create parent itemlocdist for From (transit) itemsite 
      if (recvInfo.value("recv_order_type").toString() == "TO" && _metrics->boolean("MultiWhs"))
      {
        XSqlQuery tohead;
        tohead.prepare("SELECT itemsite_id "
                    "FROM toitem "
                    "  JOIN tohead ON toitem_tohead_id = tohead_id "
                    "  JOIN itemsite ON toitem_item_id = itemsite_item_id "
                    "    AND itemsite_warehous_id = tohead_trns_warehous_id " // from wh
                    "  JOIN whsinfo ON itemsite_warehous_id = warehous_id "
                    "WHERE toitem_id = :toitem_id "
                    "  AND warehous_transit=FALSE "
                    "  AND isControlledItemsite(itemsite_id);");
        tohead.bindValue(":toitem_id", recvInfo.value("recv_orderitem_id").toInt());
        tohead.exec();
        if (tohead.first())
        {
          parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, :orderType, :orderitemId, "
           ":itemlocSeries, NULL, NULL, 'TW') AS result;");
          parentItemlocdist.bindValue(":itemsite_id", tohead.value("itemsite_id").toInt());
          parentItemlocdist.bindValue(":qty", recvInfo.value("recv_qty").toDouble() * -1);
          parentItemlocdist.bindValue(":orderType", recvInfo.value("recv_order_type").toString());
          parentItemlocdist.bindValue(":orderitemId", recvInfo.value("recv_orderitem_id").toInt());
          parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
          parentItemlocdist.exec();
          if (parentItemlocdist.first())
          {
            itemlocdistId = parentItemlocdist.value("result").toInt();
            // If the "to" itemsite is not controlled, distribute the From side here
            if (!recvInfo.value("controlled").toBool())
            {
              if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(),
                true) == XDialog::Rejected)
              {
                cleanup.exec();
                // If it's not the last item in the loop, ask the user to exit loop or continue
                if (i != (selected.size() - 1))
                {
                  if (QMessageBox::question(this,  tr("Unposted Receipts"),
                                tr("Posting distribution detail for item number %1 was cancelled but "
                                  "there are more items to post. Continue posting the remaining receipts?")
                                .arg(recvInfo.value("item_number").toString()),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
                  {
                    failedReceipts.append(id);
                    failedItems.append(recvInfo.value("item_number").toString());
                    errors.append("Detail Distribution Cancelled");
                    continue;
                  }
                  else
                  {
                    failedReceipts.append(id);
                    failedItems.append(recvInfo.value("item_number").toString());
                    errors.append("Detail Distribution Cancelled");
                    break;
                  }
                }
                else
                {
                  failedReceipts.append(id);
                  failedItems.append(recvInfo.value("item_number").toString());
                  errors.append("Detail Distribution Cancelled");
                  continue;
                }
              }
            }
          }
          else 
          {
            cleanup.exec();
            failedReceipts.append(id);
            failedItems.append(recvInfo.value("item_number").toString());
            errors.append(tr("Error Creating itemlocdist record for 'From' itemsite. %1")
              .arg(parentItemlocdist.lastError().text()));
            continue;
          }
        }
        else if (tohead.lastError().type() != QSqlError::NoError)
        {
          failedReceipts.append(id);
          failedItems.append(recvInfo.value("item_number").toString());
          errors.append(tr("Failed to retrieve transfer order item and itemsite for from warehouse. %1")
            .arg(tohead.lastError().text()));
          continue;
        }
      }
      
      // Controlled (if 'TO', this is the to itemsite)
      if (recvInfo.value("controlled").toBool() && 
        // IF Transfer Order, must be MultiWhs as well
        (recvInfo.value("recv_order_type").toString() == "TO" ? _metrics->boolean("MultiWhs") : true))
      {
        XSqlQuery parentItemlocdist;
        parentItemlocdist.prepare("SELECT createItemlocdistParent(:itemsite_id, :qty, :orderType, :orderitemId, "
          ":itemlocSeries, NULL, :itemlocdistId, :transType) AS result;");
        parentItemlocdist.bindValue(":itemsite_id", recvInfo.value("itemsite_id").toInt());
        parentItemlocdist.bindValue(":orderType", recvInfo.value("recv_order_type").toString());
        parentItemlocdist.bindValue(":orderitemId", recvInfo.value("recv_orderitem_id").toInt());
        parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
        if (itemlocdistId > 0)
          parentItemlocdist.bindValue(":itemlocdistId", itemlocdistId);
        if (recvInfo.value("recv_order_type").toString() == "TO")
        {
          parentItemlocdist.bindValue(":qty", recvInfo.value("recv_qty").toDouble());
          parentItemlocdist.bindValue(":transType", "TR");
        }
        else if (recvInfo.value("recv_order_type").toString() == "RA")
        {
          parentItemlocdist.bindValue(":qty", recvInfo.value("qty").toDouble());
          parentItemlocdist.bindValue(":transType", "RR");
        }
        else if (recvInfo.value("recv_order_type").toString() == "PO")
        {
          parentItemlocdist.bindValue(":qty", recvInfo.value("qty").toDouble());
          parentItemlocdist.bindValue(":transType", "RP");
        }
        parentItemlocdist.exec();
        if (parentItemlocdist.first())
        {
          if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
            == XDialog::Rejected)
          {
            cleanup.exec();
            // If it's not the last item in the loop, ask the user to exit loop or continue
            if (i != (selected.size() - 1))
            {
              if (QMessageBox::question(this,  tr("Unposted Receipts"),
                            tr("Posting distribution detail for item number %1 was cancelled but "
                              "there are more items to post. Continue posting the remaining receipts?")
                            .arg(recvInfo.value("item_number").toString()),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
              {
                failedReceipts.append(id);
                failedItems.append(recvInfo.value("item_number").toString());
                errors.append("Detail Distribution Cancelled");
                continue;
              }
              else
              {
                failedReceipts.append(id);
                failedItems.append(recvInfo.value("item_number").toString());
                errors.append("Detail Distribution Cancelled");
                break;
              }
            }
            else
            {
              failedReceipts.append(id);
              failedItems.append(recvInfo.value("item_number").toString());
              errors.append("Detail Distribution Cancelled");
              continue;
            }
          }
        }
        else
        {
          cleanup.exec();
          failedReceipts.append(id);
          failedItems.append(recvInfo.value("item_number").toString());
          errors.append(tr("Error creating itemlocdist record. %1")
            .arg(parentItemlocdist.lastError().text()));
          continue;
        }
      }

      // The remaining is all database, so wrap in a transaction
      unpostedPost.exec("BEGIN;");

      // Post inventory transactions starting with postReceipt
      XSqlQuery postLine;
      postLine.prepare("SELECT postReceipt(:id, :itemlocSeries, true) AS result;");
      postLine.bindValue(":id", id);
      postLine.bindValue(":itemlocSeries", itemlocSeries);
      postLine.exec();
      if (postLine.first())
      {
        int result = postLine.value("result").toInt();
        if (result < 0 || result != itemlocSeries)
        {
          rollback.exec();
          cleanup.exec();
          failedReceipts.append(id);
          failedItems.append(recvInfo.value("item_number").toString());
          errors.append(tr("Error Posting Receipt Information. %1")
            .arg(storedProcErrorLookup("postReceipt", result)));
          continue;
        }

        // Job item for Return Service; issue this to work order
        if (recvInfo.value("issuewo").toBool())
        {
          XSqlQuery issuewo;
          issuewo.prepare("SELECT issueWoRtnReceipt(coitem_order_id, invhist_id) AS result "
                          "FROM invhist, recv "
                          " JOIN raitem ON (raitem_id=recv_orderitem_id) "
                          " JOIN coitem ON (coitem_id=raitem_new_coitem_id) "
                          "WHERE ((invhist_series=:itemlocseries) "
                          " AND (recv_id=:id));");
          issuewo.bindValue(":itemlocseries", itemlocSeries);
          issuewo.bindValue(":id", id);
          issuewo.exec();
          if (issuewo.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            cleanup.exec();
            failedReceipts.append(id);
            failedItems.append(recvInfo.value("item_number").toString());
            errors.append(tr("Error posting receipt information. %1").arg(issuewo.lastError().text()));
            continue;
          }
        }
        // Issue drop ship orders to shipping
        else if (recvInfo.value("dropship").toBool())
        {
          XSqlQuery issue;
          issue.prepare("SELECT issueToShipping('SO', coitem_id, "
                        "  (recv_qty * poitem_invvenduomratio / coitem_qty_invuomratio), "
                        "  :itemlocseries, now(), invhist_id ) AS result, "
                        "  coitem_cohead_id, soHoldType(cohead_id) AS holdtype, pohead_number "
                        "FROM invhist, recv "
                        " JOIN poitem ON (poitem_id=recv_orderitem_id) "
                        " JOIN pohead ON (poitem_pohead_id=pohead_id) "
                        " JOIN coitem ON (coitem_id=poitem_order_id AND poitem_order_type='S') "
                        " JOIN cohead ON (coitem_cohead_id=cohead_id) "
                        "WHERE ((invhist_series=:itemlocseries) "
                        " AND (recv_id=:id));");
          issue.bindValue(":itemlocseries", itemlocSeries);
          issue.bindValue(":id",  id);
          issue.exec();
          if (issue.first())
          {
            if (issue.value("holdtype").toString() != "N")
            {
              QString msg = tr("Purchase Order %1 is being drop shipped against "
                       "a Sales Order that is on Hold.  The Sales Order must "
                       "be taken off Hold before the Receipt may be Posted.")
                  .arg(issue.value("pohead_number").toString());
              rollback.exec();
              cleanup.exec();
              sFillList();
              failedReceipts.append(id);
              failedItems.append(recvInfo.value("item_number").toString());
              errors.append(msg);
              continue;
            }

            if (!_soheadid.contains(issue.value("coitem_cohead_id").toInt()))
              _soheadid.append(issue.value("coitem_cohead_id").toInt());
            issue.prepare("SELECT postItemLocSeries(:itemlocseries);");
            issue.bindValue(":itemlocseries", postLine.value("result").toInt());
            issue.exec();
          }
          if (issue.lastError().type() != QSqlError::NoError)
          {
            rollback.exec();
            cleanup.exec();
            failedReceipts.append(id);
            failedItems.append(recvInfo.value("item_number").toString());
            errors.append(tr("Error posting receipt information. %1").arg(issue.lastError().text()));
            continue;
          }
        }
      }
      else if (postLine.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        cleanup.exec();
        failedReceipts.append(id);
        failedItems.append(recvInfo.value("item_number").toString());
        errors.append(tr("Error posting receipt information. %1").arg(postLine.lastError().text()));
        continue;
      }
      succeeded++;
      unpostedPost.exec("COMMIT;"); 
    } // for each selected line

    // Remove sales orders from _soheadid list that had a line item fail so that we don't ship that sales order
    for (int i = 0; i < failedReceipts.count(); i++)
    {
      XSqlQuery failedLineItems;
      failedLineItems.prepare("SELECT cohead_id "
                              "FROM recv "
                              " JOIN poitem ON poitem_id=recv_orderitem_id "
                              " JOIN pohead ON poitem_pohead_id=pohead_id "
                              " JOIN coitem ON coitem_id=poitem_order_id AND poitem_order_type='S' "
                              " JOIN cohead ON coitem_cohead_id=cohead_id "
                              "WHERE recv_id = :recv_id "
                              " AND recv_order_type = 'PO';");
      failedLineItems.bindValue(":recv_id", failedReceipts.at(i));
      failedLineItems.exec();
      if (failedLineItems.first())
      {
        _soheadid.removeAll(failedLineItems.value("cohead_id").toInt());
      }
    }

    // Ship any drop shipped orders
    while (_soheadid.count())
    {
      XSqlQuery ship;
      ship.prepare("SELECT shipShipment(shiphead_id) AS result, "
                   "  shiphead_id "
                   "FROM shiphead "
                   "WHERE ( (shiphead_order_type='SO') "
                   " AND (shiphead_order_id=:cohead_id) "
                   " AND (NOT shiphead_shipped) );");
      ship.bindValue(":cohead_id", _soheadid.at(0));
      ship.exec();
      if (_metrics->boolean("BillDropShip") && ship.first())
      {
        int shipheadid = ship.value("shiphead_id").toInt();
        ship.prepare("SELECT selectUninvoicedShipment(:shiphead_id);");
        ship.bindValue(":shiphead_id", shipheadid);
        ship.exec();
        if (ship.lastError().type() != QSqlError::NoError)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipment Information"),
                               ship, __FILE__, __LINE__);
          break;
        }
      }
      if (ship.lastError().type() != QSqlError::NoError)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipment Information"),
                             ship, __FILE__, __LINE__);
        break;
      }
      _soheadid.takeFirst();
    }

    if (errors.size() > 0)
    {
      QMessageBox dlg(QMessageBox::Critical, "Errors Posting Receipt", "", QMessageBox::Ok, this);
      dlg.setText(tr("%1 Items succeeded.\n%2 Items failed.").arg(succeeded).arg(failedItems.size()));

      QString details;
      for (int i=0; i<failedItems.size(); i++)
        details += tr("Item %1 failed with:\n%2\n").arg(failedItems[i]).arg(errors[i]);
      dlg.setDetailedText(details);

      dlg.exec();
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _recv->headerItem(), _recv->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
    else
      tryagain = false;
  } while (tryagain);

  omfgThis->sPurchaseOrderReceiptsUpdated();
}

void unpostedPoReceipts::sPopulateMenu(QMenu *pMenu,QTreeWidgetItem *pItem)
{
  QAction *menuItem;
  QString ordertype = pItem->text(_recv->column("recv_order_type"));

  menuItem = pMenu->addAction(tr("Edit Receipt..."),	this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("Delete Receipt..."),	this, SLOT(sDelete()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post Receipt..."),	this, SLOT(sPost()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Order Item..."),this, SLOT(sViewOrderItem()));
  menuItem->setEnabled(
      (ordertype == "PO" && _privileges->check("ViewPurchaseOrders")) ||
      (ordertype == "TO" && _privileges->check("ViewTransferOrders")) ||
      (ordertype == "RA" && _privileges->check("ViewReturns"))
    );
}

void unpostedPoReceipts::sFillList()
{
  ParameterList fillp;
  setParams(fillp);
  MetaSQLQuery fillm = mqlLoad("unpostedReceipts", "detail");
  XSqlQuery fillq = fillm.toQuery(fillp);

  _recv->clear();
  _recv->populate(fillq,true);
}
