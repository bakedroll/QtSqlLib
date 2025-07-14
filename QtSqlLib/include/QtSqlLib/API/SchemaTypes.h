#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/ColumnHelper.h>

#include <QVariant>

#include <map>
#include <optional>
#include <vector>

namespace QtSqlLib::API
{

using PrimaryForeignKeyColumnIdMap = std::map<IID::Type, IID::Type>;

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

using RelationshipTableIdPair = std::pair<IID::Type, IID::Type>;
using RelationshipToForeignKeyReferencesMap = std::map<RelationshipTableIdPair, std::vector<ForeignKeyReference>>;

struct Table
{
  QString name;
  std::map<IID::Type, Column> columns;
  RelationshipToForeignKeyReferencesMap relationshipToForeignKeyReferencesMap;

  ColumnHelper::SelectColumnList primaryKeys;
  ColumnHelper::SelectColumnList uniqueColIds;
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
  ColumnHelper::SelectColumnList columns;
  bool isUnique = false;
};

struct QueryMetaInfo
{
  API::IID::Type tableId = 0;
  std::optional<API::IID::Type> relationshipId;
  ColumnHelper::SelectColumnList columns;
  std::vector<size_t> columnQueryIndices;
  std::vector<size_t> primaryKeyColumnIndices;
};

}
