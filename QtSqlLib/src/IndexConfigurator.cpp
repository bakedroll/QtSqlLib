#include "IndexConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

IndexConfigurator::IndexConfigurator(API::Index& index) :
  m_index(index),
  m_columnsConfigured(false),
  m_uniqueConfigured(false)
{
}

IndexConfigurator::~IndexConfigurator() = default;

API::IIndexConfigurator& IndexConfigurator::columns(const std::vector<API::IID::Type>& columnIds)
{
  if (m_columnsConfigured)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "columns() should only be called once.");
  }

  if (columnIds.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "columns() must specify at least one column id.");
  }

  m_index.columnIds = columnIds;
  m_columnsConfigured = true;
  return *this;
}

API::IIndexConfigurator& IndexConfigurator::unique()
{
  if (m_uniqueConfigured)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, "unique() should only be called once.");
  }

  m_index.isUnique = true;
  m_uniqueConfigured = true;
  return *this;
}

}
