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
  using TableColId = std::pair<Schema::Id, Schema::Id>;
  using RelationshipParentTableId = std::pair<Schema::Id, Schema::Id>;
  using TableColumnValuesMap = std::map<TableColId, QVariant>;
  using PrimaryForeignKeyColIdMap = std::map<TableColId, Id>;

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
    PrimaryForeignKeyColIdMap primaryForeignKeyColIdMap;
  };

  struct Table
  {
    QString name;
    std::map<Id, Column> columns;
    std::map<RelationshipParentTableId, ForeignKeyReference> mapRelationshipToForeignKeyReferences;
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
  void throwIfColumnIdNotExisting(const Table& table, Id colId) const;

  Id validatePrimaryKeysAndGetTableId(const TableColumnValuesMap& columnValues) const;
  Id validatePrimaryKeysListAndGetTableId(const std::vector<TableColumnValuesMap>& columnValues) const;

  std::pair<Id, Id> validateOneToOneRelationshipPrimaryKeysAndGetTableIds(
    Schema::Id relationshipId,
    const TableColumnValuesMap& fromTableColumnValues,
    const TableColumnValuesMap& toTableColumnValues) const;

  std::pair<Id, Id> validateOneToManyRelationshipPrimaryKeysAndGetTableIds(
    Schema::Id relationshipId,
    const TableColumnValuesMap& fromTableColumnValues,
    const std::vector<TableColumnValuesMap>& toTableColumnValuesList) const;

private:
  std::map<Id, Table> m_tables;
  std::map<Id, Relationship> m_relationships;
  std::map<Id, Id> m_mapManyToManyRelationshipToLinkTableId;

  std::pair<Id, Id> validateRelationshipPrimaryKeysAndGetTableIds(
    bool bIsOneToMany,
    Schema::Id relationshipId,
    const TableColumnValuesMap& fromTableColumnValues,
    const std::vector<TableColumnValuesMap>& toTableColumnValuesList) const;

  static bool isTableIdsMatching(const Schema::Relationship& relationship, Schema::Id tableFromId,
                                 Schema::Id tableToId, bool bIgnoreFromKeys);

};

}
