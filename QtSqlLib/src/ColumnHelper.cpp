#include "QtSqlLib/ColumnHelper.h"

namespace QtSqlLib
{

ColumnHelper::GroupColumn::GroupColumn(const ColumnData& coldata) :
    data(coldata)
{
}

ColumnHelper::OrderColumn::OrderColumn(const ColumnData& coldata) :
    data(coldata)
{
}

};
