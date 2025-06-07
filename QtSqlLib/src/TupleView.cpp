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
    primaryKeyValue.columnId = m_queryMetaInfo.columns.cdata().at(columnIndex);
    primaryKeyValue.value = m_sqlQuery.value(static_cast<int>(queryIndex));
  }

  return PrimaryKey(m_queryMetaInfo.tableId, std::move(values));
}

bool TupleView::hasColumnValueIntern(const API::IID& columnId) const
{
  auto& columns = m_queryMetaInfo.columns.cdata();
  for (size_t i=0; i<columns.size(); ++i)
  {
    if (columns.at(i) == columnId.get())
    {
      return true;
    }
  }
  return false;
}

QVariant TupleView::columnValueIntern(const API::IID& columnId) const
{
  throwIfInvalidated();

  auto& columns = m_queryMetaInfo.columns.cdata();
  for (size_t i=0; i<columns.size(); ++i)
  {
    if (columns.at(i) == columnId.get())
    {
      return m_sqlQuery.value(static_cast<int>(m_queryMetaInfo.columnQueryIndices.at(i)));
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
