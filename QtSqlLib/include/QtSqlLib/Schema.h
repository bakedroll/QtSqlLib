#pragma once

#include <QVariant>

#include <map>
#include <set>

namespace QtSqlLib
{

class Schema
{
public:
  using Id = unsigned int;

  struct TableColumnId
  {
    Schema::Id tableId = 0U;
    Schema::Id columnId = 0U;

    bool operator<(const TableColumnId& rhs) const;
    bool operator!=(const TableColumnId& rhs) const;
  };

  struct RelationshipTableId
  {
    Schema::Id relationshipId = 0U;
    Schema::Id tableId = 0U;

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

  Schema();
  virtual ~Schema();

  std::map<Id, Table>& getTables();
  std::map<Id, Relationship>& getRelationships();

  Id getManyToManyLinkTableId(Id relationshipId) const;

  void configureRelationships();

  void throwIfTableIdNotExisting(Id tableId) const;
  void throwIfRelationshipIsNotExisting(Id relationshipId) const;
  static void throwIfColumnIdNotExisting(const Table& table, Id colId);

  Id validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const;
  Id validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const;

  std::pair<Id, Id> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    Schema::Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const;

  std::pair<Id, Id> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    Schema::Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const;

private:
  std::map<Id, Table> m_tables;
  std::map<Id, Relationship> m_relationships;
  std::map<Id, Id> m_mapManyToManyRelationshipToLinkTableId;

  std::pair<Id, Id> verifyRelationshipPrimaryKeysAndGetTableIds(
    bool bIsOneToMany,
    Schema::Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const;

  static bool isTableIdsMatching(const Schema::Relationship& relationship, Schema::Id tableFromId,
                                 Schema::Id tableToId, bool bIgnoreFromKeys);

};

}
