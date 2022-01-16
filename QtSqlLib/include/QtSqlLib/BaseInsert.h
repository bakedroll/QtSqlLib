#pragma once
#pragma once

#include <QtSqlLib/IQuery.h>

#include <vector>

namespace QtSqlLib
{

class BaseInsert : public IQuery
{
public:
  BaseInsert(unsigned int tableId);
  ~BaseInsert() override;

protected:
  QString getSqlQueryString(const SchemaConfigurator::Schema& schema) const;
  void addColumnId(unsigned int id);
  void checkColumnIdExisting(unsigned int id) const;

private:
  unsigned int m_tableId;
  std::vector<unsigned int> m_columnIds;

};

}