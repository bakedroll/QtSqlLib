#include "QtSqlLib/QueryIdentifiers.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/ColumnStatistics.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

static const QString columnIdentifier(
  API::ISchema& schema,
  API::IID::Type tableId,
  const std::optional<QString>& tableAlias,
  API::IID::Type columnId,
  bool bUseTableName)
{
  const auto& table = schema.getTables().at(tableId);
  schema.getSanityChecker().throwIfColumnIdNotExisting(table, columnId);
  const auto columnName = table.columns.at(columnId).name;

  if (bUseTableName)
  {
    return QString("'%1'.'%2'").arg(tableAlias.has_value() ? tableAlias.value() : table.name).arg(columnName);
  }
  return QString("'%1'").arg(columnName);
}

QueryIdentifiers::QueryIdentifiers() = default;

QueryIdentifiers::~QueryIdentifiers() = default;

void QueryIdentifiers::addTableIdentifier(
  const std::optional<API::IID::Type>& relationshipId,
  API::IID::Type tableId,
  const std::optional<QString>& tableAlias)
{
  m_tableIdentifiers.emplace_back(TableIdentifier { relationshipId, tableId, tableAlias });
}

QString QueryIdentifiers::resolveColumnIdentifier(API::ISchema& schema, const ColumnHelper::ColumnData& columnData) const
{
  for (const auto& identifier : m_tableIdentifiers)
  {
    if (identifier.relationshipId == columnData.relationshipId)
    {
      const auto columnId = columnData.columnId;
      if (ColumnStatistics::isColumnStatistics(columnId))
      {
        const auto columnStatistics = ColumnStatistics::fromId(columnId);
        if (columnStatistics.hasColumn())
        {
          return ColumnStatistics::toString(columnStatistics.type(), columnStatistics.method(),
            columnIdentifier(schema, identifier.tableId, identifier.tableAlias, columnStatistics.columnId(), m_tableIdentifiers.size() > 1));
        }
        return ColumnStatistics::toString(columnStatistics.type(), columnStatistics.method());
      }

      return columnIdentifier(schema, identifier.tableId, identifier.tableAlias, columnId, m_tableIdentifiers.size() > 1);
    }
  }

  throw DatabaseException(DatabaseException::Type::UnexpectedError, "Could not resolve column name due to unknown table identifier.");
}

}
