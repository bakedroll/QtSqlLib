#include "QtSqlLib/TupleView.h"

namespace QtSqlLib
{

TupleView::TupleView(
    const QSqlQuery& sqlQuery,
    const API::QueryMetaInfo& queryMetaInfo,
    API::IID::Type relationshipId) :
  m_sqlQuery(sqlQuery),
  m_queryMetaInfo(queryMetaInfo),
  m_relationshipId(relationshipId)
{
}

TupleView::~TupleView() = default;

API::IID::Type TupleView::tableId() const
{
  return m_queryMetaInfo.tableId;
}

API::IID::Type TupleView::relationshipId() const
{
  return m_relationshipId;
}

PrimaryKey TupleView::primaryKey() const
{
  // todo: check if all primary key indices present
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

QVariant TupleView::columnValue(const API::IID& columnId) const
{
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

}
