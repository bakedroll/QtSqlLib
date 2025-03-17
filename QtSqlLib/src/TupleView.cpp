#include "QtSqlLib/TupleView.h"

namespace QtSqlLib
{

TupleView::TupleView(
    const PrimaryKey& primaryKey,
    const ColumnList& columns,
    const std::vector<QVariant>& values,
    const std::vector<ForeignKeyRef>& foreignKeyRefs) :
  m_primaryKey(primaryKey),
  m_columns(columns),
  m_values(values),
  m_foreignKeyRefs(foreignKeyRefs)
{
}

TupleView::~TupleView() = default;

PrimaryKey TupleView::primaryKey() const
{
  return m_primaryKey;
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

PrimaryKey TupleView::foreignKey(const API::IID& relationshipId) const
{
  for (const auto& ref : m_foreignKeyRefs)
  {
    if (ref.relationshipId == relationshipId.get())
    {
      return ref.key;
    }
  }
  return {};
}

}
