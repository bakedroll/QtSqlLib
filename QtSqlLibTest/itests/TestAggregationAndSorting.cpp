#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

namespace QtSqlLibTest
{

class TestAggregationAndSorting : public testing::Test
{
public:
  TestAggregationAndSorting()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
  }

  ~TestAggregationAndSorting() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

};

static void setupTestDatabase(QtSqlLib::API::IDatabase& m_db)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Albums, "albums")
    .COLUMN(AlbumsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(AlbumsCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_TABLE(TableIds::Tracks, "tracks")
    .COLUMN(TracksCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(TracksCols::Name, "name", 128).NOT_NULL
    .COLUMN(TracksCols::Length, "length", DataType::Integer).NOT_NULL;

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumTracks, TableIds::Albums, TableIds::Tracks,
    QtSqlLib::API::RelationshipType::OneToMany);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  // Albums
  const auto album1 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 1")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto album2 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 2")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto album3 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 3")
    .RETURN_IDS).nextTuple().primaryKey();

  // Album 1 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 1)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 3)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 3")
      .VALUE(TracksCols::Length, 2)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  // Album 2 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 4)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 1)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 3")
      .VALUE(TracksCols::Length, 3)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 4")
      .VALUE(TracksCols::Length, 2)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  // Album 3 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 2)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 1)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 3")
      .VALUE(TracksCols::Length, 4)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 4")
      .VALUE(TracksCols::Length, 5)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 5")
      .VALUE(TracksCols::Length, 3)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));
}

/**
 * @test: Test the counting of album rows.
 * @expected: Expects the correct counting of rows in table 'albums'.
 */
TEST_F(TestAggregationAndSorting, countAlbums)
{
  setupTestDatabase(m_db);

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(COUNT_ALL));

  EXPECT_EQ(results.nextTuple().columnValue(COUNT_ALL).toInt(), 3);
}

}
