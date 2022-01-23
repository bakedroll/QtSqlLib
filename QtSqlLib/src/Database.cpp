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

static const Schema::Id s_sqliteMasterTableId = 0;
static const Schema::Id s_sqliteMasterTypeColId = 0;
static const Schema::Id s_sqliteMasterNameColId = 1;

static const Schema::Id s_versionColId = 0;
static const Schema::Id s_versionTableid = std::numeric_limits<Schema::Id>::max();
static const QString s_versionTableName = "database_version";

static QString getDataTypeName(Schema::DataType type, int varcharLength)
{
  switch (type)
  {
  case Schema::DataType::Integer:
    return "INTEGER";
  case Schema::DataType::Real:
    return "REAL";
  case Schema::DataType::Varchar:
    return QString("VARCHAR(%1)").arg(varcharLength);
  case Schema::DataType::Blob:
    return "BLOB";
  default:
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Unknown data type.");
  }
}

class CreateTable : public IQuery
{
public:
  CreateTable(const Schema::Table& table)
    : IQuery()
    , m_table(table)
  {}

  ~CreateTable() override = default;

  QSqlQuery getSqlQuery(Schema& schema) const override
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
  const Schema::Table& m_table;

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
    .column(s_versionColId, "version", Schema::DataType::Integer).primaryKey().notNull();

  configureSchema(configurator);
  addRelationshipsToSchema();

  loadDatabaseFile(filename);

  m_isInitialized = true;
}

void Database::close()
{
  m_db.close();
}

IQuery::QueryResults Database::execQuery(const IQuery& query)
{
  return execQuery(m_schema, query);
}

IQuery::QueryResults Database::execQuery(Schema& schema, const IQuery& query) const
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

int Database::queryDatabaseVersion()
{
  const auto results = execQuery(FromTable(s_versionTableid).select(s_versionColId));
  if (results.empty())
  {
    return -1;
  }

  return results[0].at({ s_versionTableid, s_versionColId }).toInt();
}

void Database::createOrMigrateTables(int currentVersion)
{
  const auto targetVersion = getDatabaseVersion();
  for (auto version = currentVersion+1; version <= targetVersion; version++)
  {
    if (version == 1)
    {
      for (const auto& table : m_schema.getTables())
      {
        execQuery(CreateTable(table.second));
      }

      execQuery(InsertInto(s_versionTableid).value(s_versionColId, targetVersion));
    }
  }
}

void Database::addRelationshipsToSchema()
{
  const auto checkTableIdExisting = [this](Schema::Id relId, Schema::Id tableId)
  {
    if (m_schema.getTables().count(tableId) == 0)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        QString("Relationship with id %1 references an unknown table with id %2.").arg(relId).arg(tableId));
    }
  };

  for (auto& relationship : m_schema.getRelationships())
  {
    checkTableIdExisting(relationship.first, relationship.second.tableFromId);
    checkTableIdExisting(relationship.first, relationship.second.tableToId);

    if ((relationship.second.type == Schema::RelationshipType::ManyToOne) ||
      (relationship.second.type == Schema::RelationshipType::OneToMany))
    {
      auto parentTableId = relationship.second.tableFromId;
      auto childTableId = relationship.second.tableToId;

      if (relationship.second.type == Schema::RelationshipType::ManyToOne)
      {
        std::swap(parentTableId, childTableId);
      }

      const auto& parentTable = m_schema.getTables().at(parentTableId);
      auto& childTable = m_schema.getTables().at(childTableId);

      Schema::Column parentIdColumn;
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

      Schema::Column foreignKeyColumn;
      foreignKeyColumn.name = QString("rel_%1_foreign_key").arg(relationship.first);
      foreignKeyColumn.type = parentIdColumn.type;
      foreignKeyColumn.varcharLength = parentIdColumn.varcharLength;

      childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;

      relationship.second.foreignKeyColId = nextAvailableChildTableColid;
    }
    else if (relationship.second.type == Schema::RelationshipType::ManyToMany)
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
  Schema sqliteMasterSchema;

  auto& table = sqliteMasterSchema.getTables()[s_sqliteMasterTableId];
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
