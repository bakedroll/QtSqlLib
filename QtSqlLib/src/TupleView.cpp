#include "QtSqlLib/TupleView.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

TupleView::TupleView(
    const QSqlQuery& sqlQuery,
    const API::QueryMetaInfo& queryMetaInfo) :
  m_queryPos(sqlQuery.at()),
  m_sqlQuery(sqlQuery),
  m_queryMetaInfo(queryMetaInfo)
{
}

TupleView::~TupleView() = default;

API::IID::Type TupleView::tableId() const
{
  return m_queryMetaInfo.tableId;
}

std::optional<API::IID::Type> TupleView::relationshipId() const
{
  return m_queryMetaInfo.relationshipId;
}

PrimaryKey TupleView::primaryKey() const
{
  throwIfInvalidated();

  const auto& primaryKeyIndices = m_queryMetaInfo.primaryKeyColumnIndices;
  if (primaryKeyIndices.empty())
  {
    return PrimaryKey();
  }

  std::vector<PrimaryKey::ColumnValue> values(primaryKeyIndices.size());
  for (size_t i=0; i<primaryKeyIndices.size(); ++i)
  {
    auto& primaryKeyValue = values.at(i);
    const auto columnIndex = primaryKeyIndices.at(i);
    const auto queryIndex = m_queryMetaInfo.columnQueryIndices.at(columnIndex);
    primaryKeyValue.columnId = m_queryMetaInfo.columns.at(columnIndex).column.value<API::IID::Type>();
    primaryKeyValue.value = m_sqlQuery.value(static_cast<int>(queryIndex));
  }

  return PrimaryKey(m_queryMetaInfo.tableId, std::move(values));
}

QVariant TupleView::columnValueAtIndex(size_t index) const
{
  throwIfInvalidated();
  return m_sqlQuery.value(static_cast<int>(m_queryMetaInfo.columnQueryIndices.at(index)));
}

bool TupleView::hasColumnValueIntern(const API::IID& columnId) const
{
  const auto& columns = m_queryMetaInfo.columns;
  return std::find_if(columns.cbegin(), columns.cend(), [&columnId](const ColumnHelper::SelectColumn& column) {
    return column.isColumnId(columnId.get());
  }) != columns.cend();
}

QVariant TupleView::columnValueIntern(const API::IID& columnId) const
{
  throwIfInvalidated();

  auto& columns = m_queryMetaInfo.columns;
  for (size_t i=0; i<columns.size(); ++i)
  {
    if (columns.at(i).isColumnId(columnId.get()))
    {
      return m_sqlQuery.value(static_cast<int>(m_queryMetaInfo.columnQueryIndices.at(i)));
    }
  }
  return {};
}

bool TupleView::hasAttributeValueIntern(const API::IID& attributeId) const
{
  auto& attributes = m_queryMetaInfo.attributes;
  return std::find_if(attributes.cbegin(), attributes.cend(), [&attributeId](const API::IID::Type& attribute) {
    return attribute == attributeId.get();
  }) != attributes.cend();
}

QVariant TupleView::attributeValueIntern(const API::IID& attributeId) const
{
  throwIfInvalidated();

  auto& attributes = m_queryMetaInfo.attributes;
  for (size_t i=0; i<attributes.size(); ++i)
  {
    if (attributes.at(i) == attributeId.get())
    {
      return m_sqlQuery.value(static_cast<int>(m_queryMetaInfo.attributeQueryIndices.at(i)));
    }
  }
  return {};
}

void TupleView::throwIfInvalidated() const
{
  if (m_queryPos != m_sqlQuery.at())
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Queried tuple invalidated.");
  }
}

}
