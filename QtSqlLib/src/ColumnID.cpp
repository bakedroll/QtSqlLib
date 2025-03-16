#include "QtSqlLib/ColumnID.h"

namespace QtSqlLib
{

ColumnID::ColumnID() :
  ColumnID("", std::nullopt, 0)
{
}

ColumnID::ColumnID(const API::IID& columnId) :
  ColumnID("", std::nullopt, columnId.get())
{
}

ColumnID::ColumnID(const API::IID& tableId, const API::IID& columnId)
  : ColumnID("", tableId.get(), columnId.get())
{
}

ColumnID::ColumnID(const QString& tableAlias, const API::IID& tableId, const API::IID& columnId)
  : ColumnID(tableAlias, tableId.get(), columnId.get())
{
}

ColumnID::~ColumnID() = default;

bool ColumnID::isTableIdValid() const
{
  return m_tableId.has_value();
}

API::IID::Type ColumnID::tableId() const
{
  return m_tableId.value();
}

API::IID::Type ColumnID::columnId() const
{
  return m_columnId;
}

QString ColumnID::tableAlias() const
{
  return m_tableAlias;
}

ColumnID::ColumnID(const QString& tableAlias, const std::optional<API::IID::Type>& tableId, API::IID::Type columnId) :
  m_tableAlias(tableAlias),
  m_tableId(tableId),
  m_columnId(columnId)
{
}

}
