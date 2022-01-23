#pragma once

namespace QtSqlLib
{

class RelationshipConfigurator
{
public:
  enum class RelationshipType
  {
    OneToMany,
    ManyToOne,
    ManyToMany
  };

  enum class ForeignKeyAction
  {
    NoAction,
    Restrict,
    SetNull,
    SetDefault,
    Cascade
  };

  struct Relationship
  {
    unsigned int tableFromId = 0;
    unsigned int tableToId = 0;
    RelationshipType type = RelationshipType::OneToMany;

    unsigned int foreignKeyColId = 0;

    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  };

  RelationshipConfigurator(Relationship& relationship);
  virtual ~RelationshipConfigurator();

  RelationshipConfigurator& onDelete(ForeignKeyAction action);
  RelationshipConfigurator& onUpdate(ForeignKeyAction action);

private:
  Relationship& m_relationship;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;

};

}