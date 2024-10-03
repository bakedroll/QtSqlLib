#include "CreateTable.h"

#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib::Query
{

static QString getDataTypeName(API::DataType type, int varcharLength)
{
  switch (type)
  {
  case API::DataType::Integer:
    return "INTEGER";
  case API::DataType::Real:
    return "REAL";
  case API::DataType::Varchar:
    return QString("VARCHAR(%1)").arg(varcharLength);
  case API::DataType::Blob:
    return "BLOB";
  default:
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Unknown data type.");
  }
}

static QString getActionString(API::ForeignKeyAction action)
{
  switch (action)
  {
  case API::ForeignKeyAction::NoAction:
    return "NO ACTION";
  case API::ForeignKeyAction::Restrict:
    return "RESTRICT";
  case API::ForeignKeyAction::SetNull:
    return "SET NULL";
  case API::ForeignKeyAction::SetDefault:
    return "SET DEFAULT";
  case API::ForeignKeyAction::Cascade:
    return "CASCADE";
  default:
    break;
  }

  return "";
}

CreateTable::CreateTable(const API::Table& table) :
  Query(),
  m_table(table)
{ 
}

CreateTable::~CreateTable() = default;

API::IQuery::SqlQuery CreateTable::getSqlQuery(
  const QSqlDatabase& db, API::ISchema& schema,
  const ResultSet& previousQueryResults)
{
  const auto cutTailingComma = [](QString& str)
  {
    str = str.left(str.length() - 2);
  };

  const auto listColumns = [this, &cutTailingComma](const std::set<API::IID::Type>& colIds)
  {
    QString colNames;
    for (const auto& colId : colIds)
    {
      colNames += QString("'%1', ").arg(m_table.columns.at(colId).name);
    }
    cutTailingComma(colNames);
    return colNames;
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
    if (column.second.bIsUnique)
    {
      columns += " UNIQUE";
    }
    columns.append(", ");
  }

  if (m_table.primaryKeys.size() > 1)
  {
    columns += QString("PRIMARY KEY(%1), ").arg(listColumns(m_table.primaryKeys));
  }

  if (m_table.uniqueColIds.size() > 1)
  {
    columns += QString("UNIQUE(%1), ").arg(listColumns(m_table.uniqueColIds));
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
        if (foreignKeyRef.onDeleteAction != API::ForeignKeyAction::NoAction)
        {
          onDeleteStr = QString(" ON DELETE %1").arg(getActionString(foreignKeyRef.onDeleteAction));
        }

        QString onUpdateStr;
        if (foreignKeyRef.onUpdateAction != API::ForeignKeyAction::NoAction)
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

  return { std::move(query) };
}

}
