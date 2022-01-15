#include "QtSqlLib/Database.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/InsertInto.h"
#include "QtSqlLib/IQuery.h"

#include <utilsLib/Utils.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace QtSqlLib
{

static const unsigned int s_versionColId = 0;
static const unsigned int s_versionTableid = std::numeric_limits<unsigned int>::max();

Database::Database()
  : m_isInitialized(false)
{
}

Database::~Database()
{
  if (m_db.isOpen())
  {
    m_db.close();
  }
}

void Database::initialize(const QString& filename)
{
  if (m_isInitialized)
  {
    UTILS_LOG_WARN("Database is already initialized.");
    return;
  }

  m_schema.tables[s_versionTableid] = getVersionTable();

  SchemaConfigurator configurator(m_schema);
  configureSchema(configurator);

  loadDatabaseFile(filename);

  m_isInitialized = true;
}

void Database::close()
{
  m_db.close();
}

void Database::execQuery(const IQuery& query) const
{
  auto q = query.getQueryString(m_schema);
  if (!q.exec())
  {
    const auto text = q.lastError().text();

    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Could not execute query: %1").arg(q.lastError().text()));
  }
}

void Database::loadDatabaseFile(const QString& filename)
{
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(filename);

  if (!m_db.open())
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Could not load database file: %1.").arg(filename));
  }

  QSqlQuery versionTableQuery(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='%1';")
    .arg(SchemaConfigurator::getVersionTableName()));

  if (!versionTableQuery.exec())
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      "Could not query version.");
  }

  if (!versionTableQuery.next())
  {
    createOrMigrateTables(0);
  }
  else
  {
    const auto currentVersion = queryDatabaseVersion();
    if (currentVersion <= 0)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        "Could not query version.");
    }

    const auto targetVersion = getDatabaseVersion();
    if (currentVersion < targetVersion)
    {
      createOrMigrateTables(currentVersion);
    }
    else if (currentVersion > targetVersion)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        "DB version newer than expected.");
    }
  }
}

int Database::queryDatabaseVersion() const
{
  // TODO: refactor
  QSqlQuery query(QString("SELECT version FROM %1;").arg(SchemaConfigurator::getVersionTableName()));
  if (query.exec())
  {
    while (query.next())
    {
      return query.value(0).toInt();
    }
  }

  return -1;
}

void Database::createOrMigrateTables(int currentVersion) const
{
  const auto targetVersion = getDatabaseVersion();
  for (auto version = currentVersion+1; version <= targetVersion; version++)
  {
    if (version == 1)
    {
      for (const auto& table : m_schema.tables)
      {
        queryCreateTable(table.second);
      }

      execQuery(InsertInto(s_versionTableid).value(s_versionColId, targetVersion));
    }
  }
}

int Database::getDatabaseVersion() const
{
  return 1;
}

void Database::queryCreateTable(const TableConfigurator::Table& table) const
{
  QSqlQuery query;

  QString columns;
  for (const auto& column : table.columns)
  {
    columns += QString(" '%1' %2 %3 %4 %5,")
      .arg(column.second.name)
      .arg(getDataTypeName(column.second.type, column.second.varcharLength))
      .arg(column.second.isPrimaryKey ? "PRIMARY KEY" : "")
      .arg(column.second.isAutoIncrement ? "AUTOINCREMENT" : "")
      .arg(column.second.isNotNull ? "NOT NULL" : "");
  }
  columns = columns.left(columns.length() - 1).trimmed();

  const auto queryString = QString("CREATE TABLE '%1' (%2);").arg(table.name).arg(columns);
  UTILS_LOG_DEBUG(queryString.toStdString().c_str());

  if (!query.exec(queryString))
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Could not create table %1.").arg(table.name));
  }
}

QString Database::getDataTypeName(TableConfigurator::DataType type, int varcharLength)
{
  switch (type)
  {
  case TableConfigurator::DataType::Integer:
    return "INTEGER";
  case TableConfigurator::DataType::Real:
    return "REAL";
  case TableConfigurator::DataType::Varchar:
    return QString("VARCHAR(%1)").arg(varcharLength);
  case TableConfigurator::DataType::Blob:
    return "BLOB";
  default:
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Unknown data type.");
  }
}

TableConfigurator::Table Database::getVersionTable()
{
  TableConfigurator::Column column;
  column.name = "version";
  column.type = TableConfigurator::DataType::Integer;
  column.varcharLength = 0;
  column.isPrimaryKey = true;
  column.isAutoIncrement = false;
  column.isNotNull = true;

  TableConfigurator::Table table;
  table.name = SchemaConfigurator::getVersionTableName();
  table.columns[s_versionColId] = column;

  return table;
}

}
