#pragma once

#include <QtSqlLib/IQuery.h>
#include <QtSqlLib/Expr.h>

#include <vector>

namespace QtSqlLib
{

class FromTable : public IQuery
{
public:
  FromTable(unsigned int tableId);
  ~FromTable() override;

  FromTable& select(unsigned int columnId);
  FromTable& where(Expr& expr);

  template <typename... T>
  FromTable& select(unsigned int columnId, T... args)
  {
    m_bIsSelecting = true;

    select(columnId);
    select(args...);

    m_bIsSelecting = false;
    m_bColumnsSelected = true;
    return *this;
  }

  QSqlQuery getSqlQuery(const SchemaConfigurator::Schema& schema) const override;
  QueryResults getQueryResults(const SchemaConfigurator::Schema& schema, QSqlQuery& query) const override;

private:
  unsigned int m_tableId;
  std::vector<unsigned int> m_columnIds;

  bool m_bColumnsSelected;
  bool m_bIsSelecting;

  std::unique_ptr<Expr> m_whereExpr;

};

}