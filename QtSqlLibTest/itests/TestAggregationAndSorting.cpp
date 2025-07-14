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
      .VALUE(TracksCols::Length, 5)
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
      .VALUE(TracksCols::Length, 3)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album3));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 4)
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

/**
 * @test: Test grouping by album and using aggregate functions to determine number of tracks, minimum track length and total album length.
 * @expected: Expects correct results returned by aggregate functions.
 */
TEST_F(TestAggregationAndSorting, groupByAlbums)
{
  setupTestDatabase(m_db);

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name, COUNT(AlbumsCols::Id))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id, TracksCols::Name, MIN(TracksCols::Length), SUM(TracksCols::Length))
    .GROUP_BY(AlbumsCols::Id));

  auto numRows = 0;
  while (results.hasNextTuple())
  {
    const auto& tuple = results.nextTuple();
    const auto albumName = tuple.columnValue(AlbumsCols::Name);
    const auto numTracks = tuple.columnValue(COUNT(AlbumsCols::Id)).toInt();

    const auto& joinedTuple = results.nextJoinedTuple();
    const auto minLength = joinedTuple.columnValue(MIN(TracksCols::Length)).toInt();
    const auto totalLength = joinedTuple.columnValue(SUM(TracksCols::Length)).toInt();

    if (albumName == "Album 1")
    {
      EXPECT_EQ(numTracks, 3);
      EXPECT_EQ(minLength, 1);
      EXPECT_EQ(totalLength, 6);
    }
    else if (albumName == "Album 2")
    {
      EXPECT_EQ(numTracks, 4);
      EXPECT_EQ(minLength, 2);
      EXPECT_EQ(totalLength, 14);
    }
    else if (albumName == "Album 3")
    {
      EXPECT_EQ(numTracks, 5);
      EXPECT_EQ(minLength, 3);
      EXPECT_EQ(totalLength, 19);
    }
    else
    {
      FAIL() << "Unexpected album";
    }

    ++numRows;
  }

  EXPECT_EQ(numRows, 3);
}

/**
 * @test: Test ordering of track lengths of one album.
 * @expected: Expects correct order of tracks.
 */
TEST_F(TestAggregationAndSorting, orderByTrackLength)
{
  setupTestDatabase(m_db);

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .WHERE(EQUAL(AlbumsCols::Name, "Album 1"))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id, TracksCols::Name, TracksCols::Length)
    .ORDER_BY(COL(Relationships::AlbumTracks, TracksCols::Length)));

  EXPECT_TRUE(results.hasNextTuple());

  const auto& albumTuple = results.nextTuple();
  const auto albumName = albumTuple.columnValue(AlbumsCols::Name).toString();
  EXPECT_EQ(albumName, "Album 1");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track1Tuple = results.nextJoinedTuple();
  const auto track1Name = track1Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track1Name, "Track 1");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track3Tuple = results.nextJoinedTuple();
  const auto track3Name = track3Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track3Name, "Track 3");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track2Tuple = results.nextJoinedTuple();
  const auto track2Name = track2Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track2Name, "Track 2");

  EXPECT_FALSE(results.hasNextJoinedTuple());
  EXPECT_FALSE(results.hasNextTuple());
}

/**
 * @test: Test descending ordering of track lengths of one album.
 * @expected: Expects correct order of tracks.
 */
TEST_F(TestAggregationAndSorting, orderByTrackLengthDesc)
{
  setupTestDatabase(m_db);

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .WHERE(EQUAL(AlbumsCols::Name, "Album 1"))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id, TracksCols::Name, TracksCols::Length)
    .ORDER_BY(COL(Relationships::AlbumTracks, TracksCols::Length) DESC));

  EXPECT_TRUE(results.hasNextTuple());

  const auto& albumTuple = results.nextTuple();
  const auto albumName = albumTuple.columnValue(AlbumsCols::Name).toString();
  EXPECT_EQ(albumName, "Album 1");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track2Tuple = results.nextJoinedTuple();
  const auto track2Name = track2Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track2Name, "Track 2");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track3Tuple = results.nextJoinedTuple();
  const auto track3Name = track3Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track3Name, "Track 3");

  EXPECT_TRUE(results.hasNextJoinedTuple());
  const auto& track1Tuple = results.nextJoinedTuple();
  const auto track1Name = track1Tuple.columnValue(TracksCols::Name).toString();
  EXPECT_EQ(track1Name, "Track 1");

  EXPECT_FALSE(results.hasNextJoinedTuple());
  EXPECT_FALSE(results.hasNextTuple());
}

/**
 * @test: Tests the grouping of albums that have at least 5 tracks.
 * @expected: Only 'Album 3' is returned.
 */
TEST_F(TestAggregationAndSorting, albumsHavingAtLeast5Tracks)
{
  setupTestDatabase(m_db);

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name, COUNT(AlbumsCols::Id))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id)
    .GROUP_BY(AlbumsCols::Id)
    .HAVING(GREATEREQUAL(COUNT(AlbumsCols::Id), 5)));

  EXPECT_TRUE(results.hasNextTuple());

  const auto& albumTuple = results.nextTuple();
  const auto albumName = albumTuple.columnValue(AlbumsCols::Name).toString();
  EXPECT_EQ(albumName, "Album 3");

  EXPECT_FALSE(results.hasNextTuple());
}

}
