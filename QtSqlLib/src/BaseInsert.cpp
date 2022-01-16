#include "QtSqlLib/BaseInsert.h"

#include "QtSqlLib/DatabaseException.h"

#include <utilsLib/Utils.h>

namespace QtSqlLib
{

BaseInsert::BaseInsert(unsigned int tableId)
  : m_tableId(tableId)
{
}

BaseInsert::~BaseInsert() = default;

QString BaseInsert::getSqlQueryString(const SchemaConfigurator::Schema& schema) const
{
  if (schema.tables.count(m_tableId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Invalid insert query: Unknown table with id %1.").arg(m_tableId));
  }

  const auto& table = schema.tables.at(m_tableId);

  QString columnsString;
  QString valuesString;

  for (const auto& id : m_columnIds)
  {
    if (table.columns.count(id) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidQuery,
        QString("Invalid insert query: Unknown column with id %1.").arg(id));
    }

    columnsString += QString("'%1', ").arg(table.columns.at(id).name);
    valuesString += "?, ";
  }

  columnsString = columnsString.left(columnsString.length() - 2);
  valuesString = valuesString.left(valuesString.length() - 2);

  const auto queryString = QString("INSERT INTO '%1' (%2) VALUES (%3);").arg(table.name).arg(columnsString).arg(valuesString);
  UTILS_LOG_DEBUG(queryString.toStdString().c_str());

  return queryString;
}

void BaseInsert::addColumnId(unsigned int id)
{
  m_columnIds.emplace_back(id);
}

void BaseInsert::checkColumnIdExisting(unsigned int id) const
{
  for (const auto& cid : m_columnIds)
  {
    if (cid == id)
    {
      throw DatabaseException(DatabaseException::Type::InvalidQuery,
        QString("Invalid insert query: More than one column with id %1 specified.").arg(id));
    }
  }
}

}
