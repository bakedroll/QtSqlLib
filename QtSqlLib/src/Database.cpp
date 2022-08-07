#include "QtSqlLib/Database.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/QueryPrepareVisitor.h"
#include "QtSqlLib/QueryExecuteVisitor.h"
#include "QtSqlLib/Query/FromTable.h"
#include "QtSqlLib/Query/InsertInto.h"
#include "QtSqlLib/Query/Query.h"
#include "QtSqlLib/Query/QuerySequence.h"

#include <QSqlQuery>
#include <QVariant>

#include <set>

namespace QtSqlLib
{

static const int s_defaultSchemaTargetVersion = 1;

static const API::ISchema::Id s_sqliteMasterTableId = 0;
static const API::ISchema::Id s_sqliteMasterTypeColId = 0;
static const API::ISchema::Id s_sqliteMasterNameColId = 1;

static const API::ISchema::Id s_versionColId = 0;
static const API::ISchema::Id s_versionTableid = std::numeric_limits<API::ISchema::Id>::max();
static const QString s_versionTableName = "database_version";

static void verifyPrimaryKeys(const API::ISchema::Table& table)
{
  for (const auto& key : table.primaryKeys)
  {
    auto found = false;
    for (const auto& column : table.columns)
    {
      if (column.first == key)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Invalid primary key specified for table '%1'").arg(table.name));
    }
  }
}

static QString getDataTypeName(API::ISchema::DataType type, int varcharLength)
{
  switch (type)
  {
  case API::ISchema::DataType::Integer:
    return "INTEGER";
  case API::ISchema::DataType::Real:
    return "REAL";
  case API::ISchema::DataType::Varchar:
    return QString("VARCHAR(%1)").arg(varcharLength);
  case API::ISchema::DataType::Blob:
    return "BLOB";
  default:
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Unknown data type.");
  }
}

static QString getActionString(API::ISchema::ForeignKeyAction action)
{
  switch (action)
  {
  case API::ISchema::ForeignKeyAction::NoAction:
    return "NO ACTION";
  case API::ISchema::ForeignKeyAction::Restrict:
    return "RESTRICT";
  case API::ISchema::ForeignKeyAction::SetNull:
    return "SET NULL";
  case API::ISchema::ForeignKeyAction::SetDefault:
    return "SET DEFAULT";
  case API::ISchema::ForeignKeyAction::Cascade:
    return "CASCADE";
  default:
    break;
  }

  return "";
}

class CreateTable : public Query::Query
{
public:
  CreateTable(const API::ISchema::Table& table)
    : Query()
    , m_table(table)
  {}

  ~CreateTable() override = default;

  API::IQuery::SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override
  {
    const auto cutTailingComma = [](QString& str)
    {
      str = str.left(str.length() - 2);
    };

    const auto bIsSinglePrimaryKey = (m_table.primaryKeys.size() == 1);

    QString columns;
    for (const auto& column : m_table.columns)
    {
      columns += QString("'%1' %2")
        .arg(column.second.name)
        .arg(getDataTypeName(column.second.type, column.second.varcharLength));

      if (bIsSinglePrimaryKey && (m_table.primaryKeys.count(column.first) > 0))
      {
        columns += " PRIMARY KEY";
      }
      if (column.second.bIsAutoIncrement)
      {
        columns += " AUTOINCREMENT";
      }
      if (column.second.bIsNotNull)
      {
        columns += " NOT NULL";
      }
      columns.append(", ");
    }

    if (m_table.primaryKeys.size() > 1)
    {
      QString primaryKeyNames;
      for (const auto& primaryKeyId : m_table.primaryKeys)
      {
        primaryKeyNames += QString("'%1', ").arg(m_table.columns.at(primaryKeyId).name);
      }
      cutTailingComma(primaryKeyNames);
      columns += QString("PRIMARY KEY(%1), ").arg(primaryKeyNames);
    }

    if (!m_table.relationshipToForeignKeyReferencesMap.empty())
    {
      for (const auto& foreignKeyReferences : m_table.relationshipToForeignKeyReferencesMap)
      {
        for (const auto& foreignKeyRef : foreignKeyReferences.second)
        {
          const auto& parentTable = schema.getTables().at(foreignKeyRef.referenceTableId);

          QString foreignKeyColNames;
          QString parentKeyColNames;

          for (const auto& refKeyColumn : foreignKeyRef.primaryForeignKeyColIdMap)
          {
            foreignKeyColNames += QString("%1, ").arg(m_table.columns.at(refKeyColumn.second).name);
            parentKeyColNames += QString("%1, ").arg(parentTable.columns.at(refKeyColumn.first.columnId).name);
          }
          cutTailingComma(foreignKeyColNames);
          cutTailingComma(parentKeyColNames);

          QString onDeleteStr;
          if (foreignKeyRef.onDeleteAction != API::ISchema::ForeignKeyAction::NoAction)
          {
            onDeleteStr = QString(" ON DELETE %1").arg(getActionString(foreignKeyRef.onDeleteAction));
          }

          QString onUpdateStr;
          if (foreignKeyRef.onUpdateAction != API::ISchema::ForeignKeyAction::NoAction)
          {
            onUpdateStr = QString(" ON UPDATE %1").arg(getActionString(foreignKeyRef.onUpdateAction));
          }

          columns += QString("FOREIGN KEY (%1) REFERENCES '%2'(%3)%4%5, ")
            .arg(foreignKeyColNames).arg(parentTable.name).arg(parentKeyColNames)
            .arg(onDeleteStr).arg(onUpdateStr);
        }
      }
    }

    if (columns.right(2) == ", ")
    {
      cutTailingComma(columns);
    }
    columns = columns.simplified();

    QSqlQuery query;
    query.prepare(QString("CREATE TABLE '%1' (%2);").arg(m_table.name).arg(columns));

    return { query };
  }

private:
  const API::ISchema::Table& m_table;

};

Database::Database() = default;

Database::~Database()
{
  Database::close();
}

void Database::initialize(const QString& fileName, const QString& databaseName)
{
  if (m_db)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Database is already initialized.");
  }

  m_databaseName = databaseName;

  SchemaConfigurator configurator(m_schema);
  configurator.configureTable(s_versionTableid, s_versionTableName)
    .column(s_versionColId, "version", API::ISchema::DataType::Integer).primaryKey().notNull();

  configureSchema(configurator);
  m_schema.configureRelationships();

  loadDatabaseFile(fileName);
}

void Database::close()
{
  if (m_db && m_db->isOpen())
  {
    m_db->close();
    m_db.reset();

    QSqlDatabase::removeDatabase(m_databaseName);
  }
}

API::IQuery::QueryResults Database::execQuery(API::IQueryElement& query)
{
  return execQueryForSchema(m_schema, query);
}

void Database::loadDatabaseFile(const QString& filename)
{
  m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", m_databaseName));
  m_db->setDatabaseName(filename);

  if (!m_db->open())
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

    const auto targetVersion = s_defaultSchemaTargetVersion;
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
  const auto results = execQuery(Query::FromTable(s_versionTableid).select({ s_versionColId }));
  if (results.resultTuples.empty())
  {
    return -1;
  }

  return results.resultTuples[0].values.at({ s_versionTableid, s_versionColId }).toInt();
}

void Database::createOrMigrateTables(int currentVersion)
{
  const auto targetVersion = s_defaultSchemaTargetVersion;
  for (auto version = currentVersion+1; version <= targetVersion; version++)
  {
    if (version == 1)
    {
      Query::QuerySequence sequence;
      for (const auto& table : m_schema.getTables())
      {
        verifyPrimaryKeys(table.second);
        sequence.addQuery(std::make_unique<CreateTable>(table.second));
      }

      auto query = std::make_unique<Query::InsertInto>(s_versionTableid);
      query->value(s_versionColId, targetVersion);

      sequence.addQuery(std::move(query));

      execQuery(sequence);
    }
  }
}

API::IQuery::QueryResults Database::execQueryForSchema(API::ISchema& schema, API::IQueryElement& query) const
{
  if (!m_db)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Database is not yet initialized.");
  }

  QueryPrepareVisitor prepateVisitor(schema);
  query.accept(prepateVisitor);

  QueryExecuteVisitor executeVisitor(*m_db, schema);

  QSqlDatabase::database().transaction();

  try
  {
    query.accept(executeVisitor);
  }
  catch (DatabaseException&)
  {
    QSqlDatabase::database().rollback();
    throw;
  }

  QSqlDatabase::database().commit();

  return executeVisitor.getLastQueryResults();
}

bool Database::isVersionTableExisting() const
{
  Schema sqliteMasterSchema;

  auto& table = sqliteMasterSchema.getTables()[s_sqliteMasterTableId];
  table.name = "sqlite_master";
  table.columns[s_sqliteMasterTypeColId].name = "type";
  table.columns[s_sqliteMasterNameColId].name = "name";

  const auto results = execQueryForSchema(sqliteMasterSchema,
    Query::FromTable(s_sqliteMasterTableId)
    .select({ s_sqliteMasterNameColId })
    .where(Expr()
      .equal(s_sqliteMasterTypeColId, "table")
      .and()
      .equal(s_sqliteMasterNameColId, s_versionTableName)));

  return !results.resultTuples.empty();
}

}
