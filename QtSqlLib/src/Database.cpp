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
static const QString s_versionTableName = "database_version";

static QString getDataTypeName(TableConfigurator::DataType type, int varcharLength)
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

class CreateTable : public IQuery
{
public:
  CreateTable(const TableConfigurator::Table& table)
    : IQuery()
    , m_table(table)
  {}

  ~CreateTable() override = default;

  QSqlQuery getQueryString(const SchemaConfigurator::Schema& schema) const override
  {
    QString columns;
    for (const auto& column : m_table.columns)
    {
      columns += QString(" '%1' %2 %3 %4 %5,")
        .arg(column.second.name)
        .arg(getDataTypeName(column.second.type, column.second.varcharLength))
        .arg(column.second.isPrimaryKey ? "PRIMARY KEY" : "")
        .arg(column.second.isAutoIncrement ? "AUTOINCREMENT" : "")
        .arg(column.second.isNotNull ? "NOT NULL" : "");
    }
    columns = columns.left(columns.length() - 1).simplified();

    const auto queryString = QString("CREATE TABLE '%1' (%2);").arg(m_table.name).arg(columns);
    UTILS_LOG_DEBUG(queryString.toStdString().c_str());

    QSqlQuery query;
    query.prepare(queryString);

    return query;
  }

private:
  const TableConfigurator::Table& m_table;

};

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

  SchemaConfigurator configurator(m_schema);
  configurator.configureTable(s_versionTableid, s_versionTableName)
    .column(s_versionColId, "version", TableConfigurator::DataType::Integer).primaryKey().notNull();

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
    const auto t = q.lastError().text();

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
    .arg(s_versionTableName));

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
  QSqlQuery query(QString("SELECT version FROM %1;").arg(s_versionTableName));
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
        execQuery(CreateTable(table.second));
      }

      execQuery(InsertInto(s_versionTableid).value(s_versionColId, targetVersion));
    }
  }
}

int Database::getDatabaseVersion() const
{
  return 1;
}

}
