#include "QtSqlLib/ColumnHelper.h"

namespace QtSqlLib
{

ColumnHelper::SelectColumn::SelectColumn() = default;

ColumnHelper::ColumnData::ColumnData() = default;

ColumnHelper::GroupColumn::GroupColumn() = default;

ColumnHelper::GroupColumn::GroupColumn(const ColumnData& coldata) :
    data(coldata)
{
}

ColumnHelper::OrderColumn::OrderColumn() = default;

ColumnHelper::OrderColumn::OrderColumn(const ColumnData& coldata) :
    data(coldata)
{
}

};
