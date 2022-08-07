#pragma once

#include <map>
#include <set>

#include <QVariant>

namespace QtSqlLib::API
{

class ISchema
{
public:
  using Id = unsigned int;

  struct TableColumnId
  {
    Id tableId = 0U;
    Id columnId = 0U;

    bool operator<(const TableColumnId& rhs) const;
    bool operator!=(const TableColumnId& rhs) const;
  };

  struct RelationshipTableId
  {
    Id relationshipId = 0U;
    Id tableId = 0U;

    bool operator<(const RelationshipTableId& rhs) const;
  };

  using TupleValues = std::map<TableColumnId, QVariant>;
  using PrimaryForeignKeyColumnIdMap = std::map<TableColumnId, Id>;

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
    Blob
  };

  struct Column
  {
    QString name;
    DataType type = DataType::Integer;
    int varcharLength = 0;

    bool bIsAutoIncrement = false;
    bool bIsNotNull = false;
  };

  struct ForeignKeyReference
  {
    Id referenceTableId = 0;
    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
    PrimaryForeignKeyColumnIdMap primaryForeignKeyColIdMap;
  };

  using RelationshipToForeignKeyReferencesMap = std::map<RelationshipTableId, std::vector<ForeignKeyReference>>;

  struct Table
  {
    QString name;
    std::map<Id, Column> columns;
    RelationshipToForeignKeyReferencesMap relationshipToForeignKeyReferencesMap;
    std::set<Id> primaryKeys;
  };

  enum class RelationshipType
  {
    OneToMany,
    ManyToOne,
    ManyToMany
  };

  struct Relationship
  {
    Id tableFromId = 0;
    Id tableToId = 0;
    RelationshipType type = RelationshipType::OneToMany;
    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  };

  ISchema() = default;
  virtual ~ISchema() = default;

  virtual std::map<Id, Table>& getTables() = 0;
  virtual std::map<Id, Relationship>& getRelationships() = 0;

  virtual Id getManyToManyLinkTableId(Id relationshipId) const = 0;

  virtual void configureRelationships() = 0;

  virtual void throwIfTableIdNotExisting(Id tableId) const = 0;
  virtual void throwIfRelationshipIsNotExisting(Id relationshipId) const = 0;
  virtual void throwIfColumnIdNotExisting(const Table& table, Id colId) const = 0;

  virtual Id validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const = 0;
  virtual Id validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const = 0;

  virtual std::pair<Id, Id> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const = 0;

  virtual std::pair<Id, Id> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const = 0;

};

}
