// MIT License
//
// Copyright (c) 2022 Streamlet (streamlet@outlook.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>
#include <ini.h>

namespace {

using ini_file = xl::ini_t<char>;

const char *CONTENT = "[section1]    ;section1 comment\r\n"
                      "key1 = value1 ;comment1\r\n"
                      "key2 = value2 ;comment2\r\n"
                      "\r\n"
                      "[section2]    ;section2 comment\r\n"
                      "key3 = value3 ;comment3\r\n"
                      "\r\n";

} // namespace

TEST(ini_test, normal) {
  ini_file ini;
  ASSERT_EQ(ini.parse(CONTENT), true);
  ini_file::ini_data data = ini.data();
  ASSERT_EQ(data.sections.size(), 2);
  {
    const auto &section = data.sections.front();
    ASSERT_EQ(section.name, "section1");
    ASSERT_EQ(section.comment, "section1 comment");
    ASSERT_EQ(section.lines.size(), 2);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key1");
      ASSERT_EQ(line.value, "value1");
      ASSERT_EQ(line.comment, "comment1");
    }
    {
      const auto &line = section.lines.back();
      ASSERT_EQ(line.key, "key2");
      ASSERT_EQ(line.value, "value2");
      ASSERT_EQ(line.comment, "comment2");
    }
  }
  {
    const auto &section = data.sections.back();
    ASSERT_EQ(section.name, "section2");
    ASSERT_EQ(section.comment, "section2 comment");
    ASSERT_EQ(section.lines.size(), 1);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key3");
      ASSERT_EQ(line.value, "value3");
      ASSERT_EQ(line.comment, "comment3");
    }
  }
  ASSERT_EQ(ini.dump(), CONTENT);
}

TEST(ini_test, no_ending_crlf) {
  {
    const char *content = "[section1] ;section1 comment";
    ini_file ini;
    ASSERT_EQ(ini.parse(content), true);
    ini_file::ini_data data = ini.data();
    ASSERT_EQ(data.sections.size(), 1);
    {
      const auto &section = data.sections.front();
      ASSERT_EQ(section.name, "section1");
      ASSERT_EQ(section.comment, "section1 comment");
      ASSERT_EQ(section.lines.size(), 0);
    }
    ASSERT_EQ(ini.dump(), content + std::string("\r\n\r\n"));
  }
  {
    const char *content = "[section1]    ;section1 comment\r\n"
                          "key1 = value1 ;comment1";
    ini_file ini;
    ASSERT_EQ(ini.parse(content), true);
    ini_file::ini_data data = ini.data();
    ASSERT_EQ(data.sections.size(), 1);
    {
      const auto &section = data.sections.front();
      ASSERT_EQ(section.name, "section1");
      ASSERT_EQ(section.comment, "section1 comment");
      ASSERT_EQ(section.lines.size(), 1);
      {
        const auto &line = section.lines.front();
        ASSERT_EQ(line.key, "key1");
        ASSERT_EQ(line.value, "value1");
        ASSERT_EQ(line.comment, "comment1");
      }
    }
    ASSERT_EQ(ini.dump(), content + std::string("\r\n\r\n"));
  }
  {
    const char *content = "[section1]    ;section1 comment\r\n"
                          "key1 = value1 ; comment1\r\n"
                          "\r\n"
                          "[section2] ;section2 comment";
    ini_file ini;
    ASSERT_EQ(ini.parse(content), true);
    ini_file::ini_data data = ini.data();
    ASSERT_EQ(data.sections.size(), 2);
    {
      const auto &section = data.sections.front();
      ASSERT_EQ(section.name, "section1");
      ASSERT_EQ(section.comment, "section1 comment");
      ASSERT_EQ(section.lines.size(), 1);
      {
        const auto &line = section.lines.front();
        ASSERT_EQ(line.key, "key1");
        ASSERT_EQ(line.value, "value1");
        ASSERT_EQ(line.comment, " comment1");
      }
    }
    {
      const auto &section = data.sections.back();
      ASSERT_EQ(section.name, "section2");
      ASSERT_EQ(section.comment, "section2 comment");
      ASSERT_EQ(section.lines.size(), 0);
    }
    ASSERT_EQ(ini.dump(), content + std::string("\r\n\r\n"));
  }
  {
    const char *content = "; comment line";

    ini_file ini;
    ASSERT_EQ(ini.parse(content), true);
    ini_file::ini_data data = ini.data();
    ASSERT_EQ(data.sections.size(), 1);
    {
      const auto &section = data.sections.front();
      ASSERT_EQ(section.name, "");
      ASSERT_EQ(section.comment, "");
      ASSERT_EQ(section.lines.size(), 1);
      {
        const auto &line = section.lines.front();
        ASSERT_EQ(line.key, "");
        ASSERT_EQ(line.value, "");
        ASSERT_EQ(line.comment, " comment line");
      }
    }
    ASSERT_EQ(ini.dump(), content + std::string("\r\n\r\n"));
  }
  {
    const char *content = "; comment line\r\n"
                          "key1 = value1 ; comment1";

    ini_file ini;
    ASSERT_EQ(ini.parse(content), true);
    ini_file::ini_data data = ini.data();
    ASSERT_EQ(data.sections.size(), 1);
    {
      const auto &section = data.sections.front();
      ASSERT_EQ(section.name, "");
      ASSERT_EQ(section.comment, "");
      ASSERT_EQ(section.lines.size(), 2);
      {
        const auto &line = section.lines.front();
        ASSERT_EQ(line.key, "");
        ASSERT_EQ(line.value, "");
        ASSERT_EQ(line.comment, " comment line");
      }
      {
        const auto &line = section.lines.back();
        ASSERT_EQ(line.key, "key1");
        ASSERT_EQ(line.value, "value1");
        ASSERT_EQ(line.comment, " comment1");
      }
    }
    ASSERT_EQ(ini.dump(), content + std::string("\r\n\r\n"));
  }
}

TEST(ini_test, global_section) {
  const char *content = "; comment line\r\n"
                        "key1 = value1 ; comment1\r\n"
                        "\r\n"
                        "[section2]    ; section2 comment\r\n"
                        "key2 = value2 ; comment2\r\n"
                        "\r\n";

  ini_file ini;
  ASSERT_EQ(ini.parse(content), true);
  ini_file::ini_data data = ini.data();
  ASSERT_EQ(data.sections.size(), 2);
  {
    const auto &section = data.sections.front();
    ASSERT_EQ(section.name, "");
    ASSERT_EQ(section.comment, "");
    ASSERT_EQ(section.lines.size(), 2);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "");
      ASSERT_EQ(line.value, "");
      ASSERT_EQ(line.comment, " comment line");
    }
    {
      const auto &line = section.lines.back();
      ASSERT_EQ(line.key, "key1");
      ASSERT_EQ(line.value, "value1");
      ASSERT_EQ(line.comment, " comment1");
    }
  }
  {
    const auto &section = data.sections.back();
    ASSERT_EQ(section.name, "section2");
    ASSERT_EQ(section.comment, " section2 comment");
    ASSERT_EQ(section.lines.size(), 1);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key2");
      ASSERT_EQ(line.value, "value2");
      ASSERT_EQ(line.comment, " comment2");
    }
  }
  ASSERT_EQ(ini.dump(), content);
}

TEST(ini_test, duplicated_sections_and_keys) {
  const char *content = "[section1]     ;section1 comment\r\n"
                        "key1 = value11 ;comment11\r\n"
                        "key1 = value12 ;comment12\r\n"
                        "\r\n"
                        "[section2]     ;section2 comment1\r\n"
                        "key2 = value2 ;comment2\r\n"
                        "\r\n"
                        "[section2]     ;section2 comment2\r\n"
                        "key3 = value3 ;comment3\r\n"
                        "\r\n";

  ini_file ini;
  ASSERT_EQ(ini.parse(content), true);
  ini_file::ini_data data = ini.data();
  ASSERT_EQ(data.sections.size(), 2);
  {
    const auto &section = data.sections.front();
    ASSERT_EQ(section.name, "section1");
    ASSERT_EQ(section.comment, "section1 comment");
    ASSERT_EQ(section.lines.size(), 1);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key1");
      ASSERT_EQ(line.value, "value12");
      ASSERT_EQ(line.comment, "comment12");
    }
  }
  {
    const auto &section = *(++data.sections.begin());
    ASSERT_EQ(section.name, "section2");
    ASSERT_EQ(section.comment, "section2 comment2");
    ASSERT_EQ(section.lines.size(), 2);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key2");
      ASSERT_EQ(line.value, "value2");
      ASSERT_EQ(line.comment, "comment2");
    }
    {
      const auto &line = section.lines.back();
      ASSERT_EQ(line.key, "key3");
      ASSERT_EQ(line.value, "value3");
      ASSERT_EQ(line.comment, "comment3");
    }
  }
  ASSERT_EQ(ini.dump(), "[section1]     ;section1 comment\r\n"
                        "key1 = value12 ;comment12\r\n"
                        "\r\n"
                        "[section2]    ;section2 comment2\r\n"
                        "key2 = value2 ;comment2\r\n"
                        "key3 = value3 ;comment3\r\n"
                        "\r\n");
}

TEST(ini_test, unescape) {
  const char *content = "[section1]            ;section1 comment\r\n"
                        "\" key1 \" = \" value1 \" ; comment1 \r\n"
                        "\r\n";
  ini_file ini;
  ASSERT_EQ(ini.parse(content), true);
  ini_file::ini_data data = ini.data();
  ASSERT_EQ(data.sections.size(), 1);
  {
    const auto &section = data.sections.front();
    ASSERT_EQ(section.name, "section1");
    ASSERT_EQ(section.comment, "section1 comment");
    ASSERT_EQ(section.lines.size(), 1);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, " key1 ");
      ASSERT_EQ(line.value, " value1 ");
      ASSERT_EQ(line.comment, " comment1 ");
    }
  }
  ASSERT_EQ(ini.dump(), content);
}

TEST(ini_test, dump_pretty) {
  const char *content = "[section1];section1 comment\r\n"
                        "key1=value1;comment1\r\n"
                        "key2=value2;comment2\r\n"
                        "\r\n"
                        "   [   section2   ]       ;section2 comment\r\n"
                        "   key3    =    value3    ;comment3\r\n"
                        "   \r\n   ";

  ini_file ini;
  ASSERT_EQ(ini.parse(content), true);
  ini_file::ini_data data = ini.data();
  ASSERT_EQ(data.sections.size(), 2);
  {
    const auto &section = data.sections.front();
    ASSERT_EQ(section.name, "section1");
    ASSERT_EQ(section.comment, "section1 comment");
    ASSERT_EQ(section.lines.size(), 2);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key1");
      ASSERT_EQ(line.value, "value1");
      ASSERT_EQ(line.comment, "comment1");
    }
    {
      const auto &line = section.lines.back();
      ASSERT_EQ(line.key, "key2");
      ASSERT_EQ(line.value, "value2");
      ASSERT_EQ(line.comment, "comment2");
    }
  }
  {
    const auto &section = data.sections.back();
    ASSERT_EQ(section.name, "section2");
    ASSERT_EQ(section.comment, "section2 comment");
    ASSERT_EQ(section.lines.size(), 1);
    {
      const auto &line = section.lines.front();
      ASSERT_EQ(line.key, "key3");
      ASSERT_EQ(line.value, "value3");
      ASSERT_EQ(line.comment, "comment3");
    }
  }
  ASSERT_EQ(ini.dump(), CONTENT);
}

TEST(ini_test, read_api) {
  ini_file ini;
  ASSERT_EQ(ini.parse(CONTENT), true);
  ASSERT_EQ(ini.has_section("section0"), false);
  ASSERT_EQ(ini.has_section("section1"), true);
  ASSERT_EQ(ini.has_section("section2"), true);
  ASSERT_EQ(ini.has_section("section3"), false);
  ASSERT_EQ(ini.enum_sections(), (std::vector<std::string>{"section1", "section2"}));

  ASSERT_EQ(ini.has_key("section0", "key1"), false);
  ASSERT_EQ(ini.has_key("section1", "key1"), true);
  ASSERT_EQ(ini.has_key("section2", "key1"), false);
  ASSERT_EQ(ini.has_key("section0", "key2"), false);
  ASSERT_EQ(ini.has_key("section1", "key2"), true);
  ASSERT_EQ(ini.has_key("section2", "key2"), false);
  ASSERT_EQ(ini.has_key("section0", "key3"), false);
  ASSERT_EQ(ini.has_key("section1", "key3"), false);
  ASSERT_EQ(ini.has_key("section2", "key3"), true);
  ASSERT_EQ(ini.has_key("section0", "key4"), false);
  ASSERT_EQ(ini.has_key("section1", "key4"), false);
  ASSERT_EQ(ini.has_key("section2", "key4"), false);
  ASSERT_EQ(ini.enum_keys("section0"), (std::vector<std::string>{}));
  ASSERT_EQ(ini.enum_keys("section1"), (std::vector<std::string>{"key1", "key2"}));
  ASSERT_EQ(ini.enum_keys("section2"), (std::vector<std::string>{"key3"}));
  ASSERT_EQ(ini.enum_keys("section3"), (std::vector<std::string>{}));

  ASSERT_EQ(ini.get_value("section0", "key1"), "");
  ASSERT_EQ(ini.get_value("section1", "key1"), "value1");
  ASSERT_EQ(ini.get_value("section2", "key1"), "");
  ASSERT_EQ(ini.get_value("section0", "key2"), "");
  ASSERT_EQ(ini.get_value("section1", "key2"), "value2");
  ASSERT_EQ(ini.get_value("section2", "key2"), "");
  ASSERT_EQ(ini.get_value("section0", "key3"), "");
  ASSERT_EQ(ini.get_value("section1", "key3"), "");
  ASSERT_EQ(ini.get_value("section2", "key3"), "value3");
  ASSERT_EQ(ini.get_value("section0", "key4"), "");
  ASSERT_EQ(ini.get_value("section1", "key4"), "");
  ASSERT_EQ(ini.get_value("section2", "key4"), "");

  ASSERT_EQ(ini.dump(), CONTENT);
}

TEST(ini_test, write_api) {
  ini_file ini;
  ini.add_section("section1", "section1 comment");
  ini.set_value("section1", "key1", "value1", "comment1");
  ini.set_value("section1", "key2", "value2", "comment2");
  ini.add_section("section2", "section2 comment");
  ini.set_value("section2", "key3", "value3", "comment3");
  ASSERT_EQ(ini.dump(), CONTENT);
}