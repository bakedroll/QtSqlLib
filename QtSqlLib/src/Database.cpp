#include "QtSqlLib/Database.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/FromTable.h"
#include "QtSqlLib/InsertInto.h"
#include "QtSqlLib/IQuery.h"

#include <utilsLib/Utils.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <set>

namespace QtSqlLib
{

static const unsigned int s_sqliteMasterTableId = 0;
static const unsigned int s_sqliteMasterTypeColId = 0;
static const unsigned int s_sqliteMasterNameColId = 1;

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

  QSqlQuery getSqlQuery(const SchemaConfigurator::Schema& schema) const override
  {
    QString columns;
    for (const auto& column : m_table.columns)
    {
      columns += QString(" '%1' %2 %3 %4 %5,")
        .arg(column.second.name)
        .arg(getDataTypeName(column.second.type, column.second.varcharLength))
        .arg(column.second.bIsPrimaryKey ? "PRIMARY KEY" : "")
        .arg(column.second.bIsAutoIncrement ? "AUTOINCREMENT" : "")
        .arg(column.second.bIsNotNull ? "NOT NULL" : "");
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
  addRelationshipsToSchema();

  loadDatabaseFile(filename);

  m_isInitialized = true;
}

void Database::close()
{
  m_db.close();
}

IQuery::QueryResults Database::execQuery(const IQuery& query) const
{
  return execQuery(m_schema, query);
}

IQuery::QueryResults Database::execQuery(const SchemaConfigurator::Schema& schema, const IQuery& query) const
{
  auto q = query.getSqlQuery(schema);
  const auto isBatch = query.isBatchExecution();

  if ((!isBatch && !q.exec()) || (isBatch && !q.execBatch()))
  {
    throw DatabaseException(DatabaseException::Type::InvalidQuery,
      QString("Could not execute query: %1").arg(q.lastError().text()));
  }

  return query.getQueryResults(schema, q);
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

  if (!isVersionTableExisting())
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
  const auto results = execQuery(FromTable(s_versionTableid).select(s_versionColId));
  if (results.empty())
  {
    return -1;
  }

  return results[0].at({ s_versionTableid, s_versionColId }).toInt();
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

void Database::addRelationshipsToSchema()
{
  const auto checkTableIdExisting = [this](unsigned int relId, unsigned int tableId)
  {
    if (m_schema.tables.count(tableId) == 0)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        QString("Relationship with id %1 references an unknown table with id %2.").arg(relId).arg(tableId));
    }
  };

  for (auto& relationship : m_schema.relationships)
  {
    checkTableIdExisting(relationship.first, relationship.second.tableFromId);
    checkTableIdExisting(relationship.first, relationship.second.tableToId);

    if ((relationship.second.type == RelationshipConfigurator::RelationshipType::ManyToOne) ||
      (relationship.second.type == RelationshipConfigurator::RelationshipType::OneToMany))
    {
      auto parentTableId = relationship.second.tableFromId;
      auto childTableId = relationship.second.tableToId;

      if (relationship.second.type == RelationshipConfigurator::RelationshipType::ManyToOne)
      {
        std::swap(parentTableId, childTableId);
      }

      const auto& parentTable = m_schema.tables.at(parentTableId);
      auto& childTable = m_schema.tables.at(childTableId);

      TableConfigurator::Column parentIdColumn;
      for (const auto& col : parentTable.columns)
      {
        if (col.second.bIsPrimaryKey)
        {
          parentIdColumn = col.second;
          break;
        }
      }

      if (parentIdColumn.name.isEmpty())
      {
        throw DatabaseException(DatabaseException::Type::UnableToLoad,
          QString("Relationship with id %1 expects the table '%2' to have a primary key column").arg(relationship.first).arg(parentTable.name));
      }

      auto nextAvailableChildTableColid = 0U;
      while (childTable.columns.count(nextAvailableChildTableColid) > 0)
      {
        nextAvailableChildTableColid++;
      }

      TableConfigurator::Column foreignKeyColumn;
      foreignKeyColumn.name = QString("rel_%1_foreign_key").arg(relationship.first);
      foreignKeyColumn.type = parentIdColumn.type;
      foreignKeyColumn.varcharLength = parentIdColumn.varcharLength;

      childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;

      relationship.second.foreignKeyColId = nextAvailableChildTableColid;
    }
    else if (relationship.second.type == RelationshipConfigurator::RelationshipType::ManyToMany)
    {
      
    }
  }
}

int Database::getDatabaseVersion()
{
  return 1;
}

bool Database::isVersionTableExisting() const
{
  SchemaConfigurator::Schema sqliteMasterSchema;

  auto& table = sqliteMasterSchema.tables[s_sqliteMasterTableId];
  table.name = "sqlite_master";
  table.columns[s_sqliteMasterTypeColId].name = "type";
  table.columns[s_sqliteMasterNameColId].name = "name";

  const auto results = execQuery(sqliteMasterSchema,
    FromTable(s_sqliteMasterTableId)
      .select(s_sqliteMasterNameColId)
      .where(Expr()
        .equal(s_sqliteMasterTypeColId, "table")
        .and()
        .equal(s_sqliteMasterNameColId, s_versionTableName)));

  return !results.empty();
}

}
