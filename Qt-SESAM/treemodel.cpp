/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "groupnode.h"
#include "domainnode.h"
#include "treemodel.h"
#include "util.h"


TreeModel::TreeModel(QObject *parent)
  : QAbstractItemModel(parent)
  , mRootItem(Q_NULLPTR)
{
  /* ... */
}


TreeModel::~TreeModel()
{
  SafeDelete(mRootItem);
}


GroupNode *TreeModel::findChild(const QString &name, GroupNode *node) {
  GroupNode *foundChild = Q_NULLPTR;
  for (int row = 0; row < node->childCount(); ++row) {
    AbstractTreeNode *child = node->child(row);
    if (child->type() == AbstractTreeNode::GroupType) {
      GroupNode *groupNode = reinterpret_cast<GroupNode*>(child);
      if (groupNode->name() == name) {
        foundChild = groupNode;
        break;
      }
    }
  }
  return foundChild;
}


GroupNode *TreeModel::addToHierarchy(const QStringList &groups, GroupNode *node) {
  GroupNode *nextNode = node;
  foreach (QString groupName, groups) {
    nextNode = findChild(groupName, nextNode);
    if (nextNode == Q_NULLPTR) {
      nextNode = new GroupNode(groupName, node);
      node->appendChild(nextNode);
      node = nextNode;
    }
  }
  return nextNode;
}


void TreeModel::setData(const DomainSettingsList &domainSettingsList)
{
  SafeRenew<GroupNode*>(mRootItem, new GroupNode);
  foreach (DomainSettings ds, domainSettingsList) {
    if (!ds.deleted) {
      GroupNode *node = addToHierarchy(ds.groupHierarchy, mRootItem);
      node->appendChild(new DomainNode(ds, node));
    }
  }
}


int TreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return reinterpret_cast<AbstractTreeNode*>(parent.internalPointer())->columnCount();
  else
    return mRootItem->columnCount();
}


QVariant TreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  AbstractTreeNode *item = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
  return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;
  return QAbstractItemModel::flags(index);
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0:
      return tr("Domain");
    case 1:
      return tr("User");
    case 2:
      return tr("URL");
    case 3:
      return tr("Group");
    default:
      break;
    }
  }
  return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  AbstractTreeNode *parentItem;
  if (!parent.isValid())
    parentItem = mRootItem;
  else
    parentItem = reinterpret_cast<AbstractTreeNode*>(parent.internalPointer());
  AbstractTreeNode *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();
  AbstractTreeNode *childItem = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
  AbstractTreeNode *parentItem = childItem->parentItem();
  if (parentItem == mRootItem)
    return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}


int TreeModel::rowCount(const QModelIndex &parent) const
{
  AbstractTreeNode *parentItem;
  if (parent.column() > 0)
    return 0;
  if (!parent.isValid())
    parentItem = mRootItem;
  else
    parentItem = reinterpret_cast<AbstractTreeNode*>(parent.internalPointer());
  return parentItem->childCount();
}