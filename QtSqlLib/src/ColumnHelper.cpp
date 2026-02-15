#include "QtSqlLib/ColumnHelper.h"

#include "QtSqlLib/ConcatenatedColumn.h"

namespace QtSqlLib
{

ColumnHelper::ColumnAlias::ColumnAlias() = default;

ColumnHelper::ColumnAlias::ColumnAlias(const QString& alias) :
  alias(alias)
{
}

ColumnHelper::SelectColumn::SelectColumn() = default;

ColumnHelper::SelectColumn::SelectColumn(const ConcatenatedColumn& concatenatedColumn, const QString& alias) :
  column(QVariant::fromValue(concatenatedColumn)),
  alias(alias)
{
}

bool ColumnHelper::SelectColumn::isColumnId(API::IID::Type columnId) const
{
  return column.canConvert<API::IID::Type>() && column.value<API::IID::Type>() == columnId;
}

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
