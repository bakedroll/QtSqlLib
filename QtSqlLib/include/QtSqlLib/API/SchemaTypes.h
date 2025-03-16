#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/ColumnList.h>

#include <QVariant>

#include <map>
#include <set>
#include <vector>

namespace QtSqlLib::API
{

struct TableColumnId
{
  IID::Type tableId = 0;
  IID::Type columnId = 0;

  bool operator<(const TableColumnId& rhs) const;
  bool operator!=(const TableColumnId& rhs) const;
};

using PrimaryForeignKeyColumnIdMap = std::map<TableColumnId, IID::Type>;

enum class ForeignKeyAction
{
  NoAction,
  Restrict,
  SetNull,
  SetDefault,
  Cascade
};

enum class DataType
{
  Integer,
  Real,
  Varchar,
  Text,
  Blob
};

struct Column
{
  QString name;
  DataType type = DataType::Integer;
  int varcharLength = 0;

  bool bIsAutoIncrement = false;
  bool bIsNotNull = false;
  bool bIsUnique = false;
};

struct ForeignKeyReference
{
  IID::Type referenceTableId = 0;
  ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
  ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  PrimaryForeignKeyColumnIdMap primaryForeignKeyColIdMap;
};

struct RelationshipTableId
{
  IID::Type relationshipId = 0;
  IID::Type tableId = 0;

  bool operator<(const RelationshipTableId& rhs) const;
};

using RelationshipToForeignKeyReferencesMap = std::map<RelationshipTableId, std::vector<ForeignKeyReference>>;

struct Table
{
  QString name;
  std::map<IID::Type, Column> columns;
  RelationshipToForeignKeyReferencesMap relationshipToForeignKeyReferencesMap;

  // TODO: vector
  std::set<IID::Type> primaryKeys;
  std::set<IID::Type> uniqueColIds;
};

enum class RelationshipType
{
  OneToMany,
  ManyToOne,
  ManyToMany
};

struct Relationship
{
  IID::Type tableFromId = 0;
  IID::Type tableToId = 0;
  RelationshipType type = RelationshipType::OneToMany;
  ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
  ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  bool bForeignKeyIndexingEnabled = false;
};

struct Index
{
  IID::Type tableId = 0;
  QString name;
  ColumnList columns;
  bool isUnique = false;
};

using TupleValues = std::map<TableColumnId, QVariant>;

}
