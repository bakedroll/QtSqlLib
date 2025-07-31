#include "QtSqlLib/QueryIdentifiers.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/ColumnStatistics.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

static const QString columnIdentifier(const QString& tableName, const QString& columnName)
{
  if (tableName.isEmpty())
  {
    return QString("'%1'").arg(columnName);
  }

  return QString("'%1'.'%2'").arg(tableName).arg(columnName);
}

QueryIdentifiers::QueryIdentifiers() = default;

QueryIdentifiers::~QueryIdentifiers() = default;

void QueryIdentifiers::addTableIdentifier(
  const std::optional<API::IID::Type>& relationshipId,
  API::IID::Type tableId,
  const QString& tableName)
{
  m_tableIdentifiers.emplace_back(TableIdentifier { relationshipId, tableId, tableName });
}

QString QueryIdentifiers::resolveColumnIdentifier(API::ISchema& schema, const ColumnHelper::ColumnData& columnData) const
{
  for (const auto& identifier : m_tableIdentifiers)
  {
    if (identifier.relationshipId == columnData.relationshipId)
    {
      const auto& table = schema.getTables().at(identifier.tableId);
      const auto columnId = columnData.columnId;

      if (ColumnStatistics::isColumnStatistics(columnId))
      {
        const auto columnStatistics = ColumnStatistics::fromId(columnId);
        schema.getSanityChecker().throwIfColumnIdNotExisting(table, columnStatistics.columnId());

        if (columnStatistics.hasColumn())
        {
          const auto columnName = table.columns.at(columnStatistics.columnId()).name;
          return ColumnStatistics::toString(columnStatistics.type(), columnStatistics.method(),
            columnIdentifier(identifier.tableName, columnName));
        }
        return ColumnStatistics::toString(columnStatistics.type(), columnStatistics.method());
      }

      schema.getSanityChecker().throwIfColumnIdNotExisting(table, columnId);
      const auto columnName = table.columns.at(columnData.columnId).name;
      return columnIdentifier(identifier.tableName, columnName);
    }
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Could not resolve column name due to unknown table identifier.");
}

}
