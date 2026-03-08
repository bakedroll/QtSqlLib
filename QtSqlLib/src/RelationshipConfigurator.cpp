#include "RelationshipConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

#include <QString>

namespace QtSqlLib
{

RelationshipConfigurator::RelationshipConfigurator(API::Relationship& relationship) :
  m_relationship(relationship),
  m_bOnDeleteCalled(false),
  m_bOnUpdateCalled(false),
  m_bEnableForeignKeyIndexingCalled(false)
{
}

RelationshipConfigurator::~RelationshipConfigurator() = default;

API::IRelationshipConfigurator& RelationshipConfigurator::onDelete(API::ForeignKeyAction action)
{
  if (m_bOnDeleteCalled)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("onDelete() should only be called once for a relationship."));
  }

  m_relationship.onDeleteAction = action;

  m_bOnDeleteCalled = true;
  return *this;
}

API::IRelationshipConfigurator& RelationshipConfigurator::onUpdate(API::ForeignKeyAction action)
{
  if (m_bOnUpdateCalled)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("onUpdate() should only be called once for a relationship."));
  }

  m_relationship.onUpdateAction = action;

  m_bOnUpdateCalled = true;
  return *this;
}

API::IRelationshipConfigurator& RelationshipConfigurator::enableForeignKeyIndexing()
{
  if (m_bEnableForeignKeyIndexingCalled)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("enableForeignKeyIndexing() should only be called once for a relationship."));
  }

  m_relationship.bForeignKeyIndexingEnabled = true;

  m_bEnableForeignKeyIndexingCalled = true;
  return *this;
}

API::IRelationshipConfigurator& RelationshipConfigurator::attribute(
  const API::IID& attributeId, const QString& attributeName, API::DataType type, int varcharLength)
{
  if (m_relationship.type != API::RelationshipType::ManyToMany)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Attributes can only be defined for many-to-many relationships."));
  }

  const auto aid = attributeId.get();
  if ((aid & API::IID::sc_columnIdReservedBits) != 0x0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Attribute with id %1 exceeds the maximum allowed range of 27 bits.").arg(aid));
  }

  if (m_relationship.attributes.count(aid) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Attribute with id %1 already exists.").arg(aid));
  }

  if (attributeName.isEmpty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Attribute name must not be empty."));
  }

  if (isAttributeNameExisting(attributeName))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Attribute with name '%1' already exists.").arg(attributeName));
  }

  if ((type == API::DataType::Varchar) && (varcharLength <= 0))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Varchar length must greater that 0 for attribute '%1'.").arg(attributeName));
  }

  API::Attribute attribute;
  attribute.name = attributeName;
  attribute.type = type;
  attribute.varcharLength = varcharLength;
  attribute.bIsNotNull = false;

  m_relationship.attributes[aid] = attribute;
  m_lastAttributeId = aid;

  return *this;
}

API::IRelationshipConfigurator& RelationshipConfigurator::notNull()
{
  checkAttribute();
  auto& attr = m_relationship.attributes.at(m_lastAttributeId.value());

  if (attr.bIsNotNull)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("notNull() should only be called once for attribute '%1'").arg(attr.name));
  }

  attr.bIsNotNull = true;
  return *this;
}

bool RelationshipConfigurator::isAttributeNameExisting(const QString& name) const
{
  return std::find_if(m_relationship.attributes.cbegin(), m_relationship.attributes.cend(), [&name](const std::pair<API::IID::Type, API::Attribute>& attribute) {
    return attribute.second.name.toLower() == name.toLower();
  }) != m_relationship.attributes.cend();
}

void RelationshipConfigurator::checkAttribute() const
{
  if (!m_lastAttributeId.has_value())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "No attribute defined");
  }
}

}
