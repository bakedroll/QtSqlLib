#pragma once

#include <QString>

#include <map>
#include <set>
#include <vector>

namespace QtSqlLib
{

class Schema
{
public:
  using Id = unsigned int;

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
    std::map<Id, Id> mapReferenceParentColIdToChildColId;
  };

  struct Table
  {
    QString name;
    std::map<Id, Column> columns;
    std::vector<ForeignKeyReference> foreignKeyReferences;
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

  void configureRelationships();

private:
  std::map<Id, Table> m_tables;
  std::map<Id, Relationship> m_relationships;
  std::map<Id, Id> m_mapManyToManyRelationshipToLinkTableId;

};

}