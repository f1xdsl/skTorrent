#include <gtest/gtest.h>
#include <Utils/BencodeParser.hpp>

using namespace Torrent::Utils::Bencode;

TEST(BencodeTest, ParseInteger)
{
    Parser p("i42e");
    Value v = p.parse();
    EXPECT_TRUE(v.isInt());
    EXPECT_EQ(v.asInt(), 42);
}

TEST(BencodeTest, ParseNegativeInteger)
{
    Parser p("i-123e");
    Value v = p.parse();
    EXPECT_TRUE(v.isInt());
    EXPECT_EQ(v.asInt(), -123);
}

TEST(BencodeTest, ParseString)
{
    Parser p("4:spam");
    Value v = p.parse();
    EXPECT_TRUE(v.isStr());
    EXPECT_EQ(v.asStr(), "spam");
}

TEST(BencodeTest, ParseList)
{
    Parser p("l4:spam4:eggse");
    Value v = p.parse();
    ASSERT_TRUE(v.isList());
    auto& list = v.asList();
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].asStr(), "spam");
    EXPECT_EQ(list[1].asStr(), "eggs");
}

TEST(BencodeTest, ParseDict)
{
    Parser p("d3:bar4:spam3:fooi42ee");
    Value v = p.parse();
    ASSERT_TRUE(v.isDict());
    auto& dict = v.asDict();
    EXPECT_EQ(dict.at("bar").asStr(), "spam");
    EXPECT_EQ(dict.at("foo").asInt(), 42);
}

TEST(BencodeTest, Nested)
{
    Parser p("d4:listli1ei2ee3:str5:helloe");

    Value v = p.parse();
    ASSERT_TRUE(v.isDict());
    auto& dict = v.asDict();

    ASSERT_TRUE(dict.at("list").isList());
    auto& list = dict.at("list").asList();
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].asInt(), 1);
    EXPECT_EQ(list[1].asInt(), 2);

    ASSERT_TRUE(dict.at("str").isStr());
    EXPECT_EQ(dict.at("str").asStr(), "hello");
}
