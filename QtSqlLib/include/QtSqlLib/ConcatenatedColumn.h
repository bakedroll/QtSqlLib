#pragma once

#include <QtSqlLib/API/IQueryIdentifiers.h>
#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ColumnHelper.h>

#include <vector>

#include <QVariant>

namespace QtSqlLib
{

class ConcatenatedColumn
{
public:
  ConcatenatedColumn();

  template<typename... Args>
  ConcatenatedColumn(const Args&... args) :
    m_elements(sizeof...(Args))
  {
    makeElements(0, args...);
  }

  QString buildString(const API::IQueryIdentifiers& queryIdentifiers, API::ISchema& schema) const;

private:
    std::vector<QVariant> m_elements;

    template<typename TID, typename... Args>
    void makeElements(size_t currentIndex, const TID& id, const Args&... tail)
    {
      m_elements[currentIndex] = QVariant::fromValue(ColumnHelper::ColumnData(id));
      makeElements(currentIndex + 1, tail...);
    }

    template<typename TID>
    void makeElements(size_t currentIndex, const TID& id)
    {
      m_elements[currentIndex] = QVariant::fromValue(ColumnHelper::ColumnData(id));
    }

    template<typename... Args>
    void makeElements(size_t currentIndex, const QString& str, const Args&... tail)
    {
      m_elements[currentIndex] = str;
      makeElements(currentIndex + 1, tail...);
    }

    void makeElements(size_t currentIndex, const QString& str);

    template<typename... Args>
    void makeElements(size_t currentIndex, const char* str, const Args&... tail)
    {
      m_elements[currentIndex] = QString(str);
      makeElements(currentIndex + 1, tail...);
    }

    void makeElements(size_t currentIndex, const char* str);

};

}

Q_DECLARE_METATYPE(QtSqlLib::ConcatenatedColumn);