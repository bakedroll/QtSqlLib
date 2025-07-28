#include "QtSqlLib/ColumnHelper.h"

namespace QtSqlLib
{

ColumnHelper::GroupColumn::GroupColumn(const GroupColumn& rhs) :
    relationshipId(rhs.relationshipId),
    columnId(rhs.columnId)
{
}

ColumnHelper::GroupColumn::GroupColumn(GroupColumn&& rhs) :
    relationshipId(std::move(rhs.relationshipId)),
    columnId(std::move(rhs.columnId))
{
}

void ColumnHelper::GroupColumn::operator=(const GroupColumn& rhs)
{
    relationshipId = rhs.relationshipId;
    columnId = rhs.columnId;
}

ColumnHelper::OrderColumn::OrderColumn(const OrderColumn& rhs) :
    relationshipId(rhs.relationshipId),
    columnId(rhs.columnId),
    order(rhs.order)
{
}

ColumnHelper::OrderColumn::OrderColumn(OrderColumn&& rhs) :
    relationshipId(std::move(rhs.relationshipId)),
    columnId(std::move(rhs.columnId)),
    order(std::move(rhs.order))
{
}

void ColumnHelper::OrderColumn::operator=(const OrderColumn& rhs)
{
    relationshipId = rhs.relationshipId;
    columnId = rhs.columnId;
    order = rhs.order;
}

};
