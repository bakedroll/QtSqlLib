#include "QtSqlLib/ColumnID.h"

namespace QtSqlLib
{

ColumnID::ColumnID()
  : ColumnID("", {0U, 0U}, false)
{
}

ColumnID::ColumnID(const API::TableColumnId& tableColumnId)
  : ColumnID("", tableColumnId, true)
{
}

ColumnID::ColumnID(const API::IID& columnId)
  : ColumnID("", {0U, columnId.get()}, false)
{
}

ColumnID::ColumnID(const QString& tableAlias, const API::TableColumnId& tableColumnId)
  : ColumnID(tableAlias, tableColumnId, true)
{
}

ColumnID::~ColumnID() = default;

const API::TableColumnId& ColumnID::get() const
{
  return m_tableColumnId;
}

bool ColumnID::isTableIdValid() const
{
  return m_bIsTableIdValid;
}

QString ColumnID::getTableAlias() const
{
  return m_tableAlias;
}

ColumnID::ColumnID(const QString& tableAlias, const API::TableColumnId& tableColumnId, bool bIsTableIdValid) :
  m_tableAlias(tableAlias),
  m_tableColumnId(tableColumnId),
  m_bIsTableIdValid(bIsTableIdValid)
{
}

}
