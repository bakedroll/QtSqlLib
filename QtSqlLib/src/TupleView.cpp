#include "QtSqlLib/TupleView.h"

namespace QtSqlLib
{

TupleView::TupleView(
    API::IID::Type tableId,
    const std::vector<size_t>& primaryKeyColumnIndices,
    const ColumnList& columns,
    const std::vector<QVariant>& values) :
  m_tableId(tableId),
  m_primaryKeyColumnIndices(primaryKeyColumnIndices),
  m_columns(columns),
  m_values(values)
{
}

TupleView::~TupleView() = default;

API::IID::Type TupleView::tableId() const
{
  return m_tableId;
}

PrimaryKey TupleView::primaryKey() const
{
  if (m_primaryKeyColumnIndices.empty())
  {
    return PrimaryKey();
  }

  std::vector<PrimaryKey::ColumnValue> values(m_primaryKeyColumnIndices.size());
  for (size_t i=0; i<m_primaryKeyColumnIndices.size(); ++i)
  {
    auto& primaryKeyValue = values.at(i);
    const auto columnIndex = m_primaryKeyColumnIndices.at(i);
    primaryKeyValue.columnId = m_columns.cdata().at(columnIndex);
    primaryKeyValue.value = m_values.at(columnIndex);
  }

  return PrimaryKey(m_tableId, std::move(values));
}

const std::vector<QVariant>& TupleView::values() const
{
  return m_values;
}

QVariant TupleView::columnValue(const API::IID& columnId) const
{
  auto& columns = m_columns.cdata();
  for (size_t i=0; i<columns.size(); ++i)
  {
    if (columns.at(i) == columnId.get())
    {
      return m_values.at(i);
    }
  }
  return {};
}

/*PrimaryKey TupleView::foreignKey(const API::IID& relationshipId) const
{
  for (const auto& ref : m_foreignKeyRefs)
  {
    if (ref.relationshipId == relationshipId.get())
    {
      return ref.key;
    }
  }
  return {};
}*/

}
