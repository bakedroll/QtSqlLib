#include "QtSqlLib/InsertInto.h"

#include "QtSqlLib/DatabaseException.h"

#include <utilsLib/Utils.h>

namespace QtSqlLib
{
InsertInto::InsertInto(unsigned int tableId)
  : IQuery()
  , m_tableId(tableId)
{
}

InsertInto::~InsertInto() = default;

InsertInto& InsertInto::value(unsigned columnId, const QVariant& value)
{
  if (m_values.count(columnId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Invalid insert query: More than one column with id %1 specified.").arg(columnId));
  }

  m_values[columnId] = value;
  return *this;
}

QSqlQuery InsertInto::getQueryString(const SchemaConfigurator::Schema& schema) const
{
  if (schema.tables.count(m_tableId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Invalid insert query: Unknown table with id %1.").arg(m_tableId));
  }

  const auto& table = schema.tables.at(m_tableId);

  QString columnsString;
  QString valuesString;

  QSqlQuery query;

  for (const auto& value : m_values)
  {
    if (table.columns.count(value.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidQuery,
        QString("Invalid insert query: Unknown column with id %1.").arg(value.first));
    }

    columnsString += QString("'%1', ").arg(table.columns.at(value.first).name);
    valuesString += "?, ";
  }

  columnsString = columnsString.left(columnsString.length() - 2);
  valuesString = valuesString.left(valuesString.length() - 2);

  const auto queryString = QString("INSERT INTO '%1' (%2) VALUES (%3);").arg(table.name).arg(columnsString).arg(valuesString);
  UTILS_LOG_DEBUG(queryString.toStdString().c_str());

  query.prepare(queryString);

  for (const auto& value : m_values)
  {
    query.addBindValue(value.second);
  }

  return query;
}

}
