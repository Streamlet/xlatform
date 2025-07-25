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

#include <algorithm>
#include <sstream>
#include <xl/encoding>
#include <xl/file>
#include <xl/ini>
#include <xl/string>

namespace xl {

template <typename CharType>
class ini_t {
public:
  struct ini_line {
    std::basic_string<CharType> key;
    std::basic_string<CharType> value;
    std::basic_string<CharType> comment;
  };
  struct ini_section {
    std::basic_string<CharType> name;
    std::basic_string<CharType> comment;
    std::list<ini_line> lines;
    std::map<std::basic_string<CharType>, typename std::list<ini_line>::iterator> line_map;
  };
  struct ini_data {
    std::list<ini_section> sections;
    std::map<std::basic_string<CharType>, typename std::list<ini_section>::iterator> section_map;
  };

public:
  bool load(const TCHAR *path);
  bool save(const TCHAR *path) const;

  bool parse(const std::basic_string<CharType> &content);
  std::basic_string<CharType> dump() const;

  const ini_data &data() const;

  std::vector<std::basic_string<CharType>> enum_sections() const;
  bool has_section(const std::basic_string<CharType> &section) const;
  bool add_section(const std::basic_string<CharType> &section, const std::basic_string<CharType> &comment = {});
  bool remove_section(const std::basic_string<CharType> &section);

  std::vector<std::basic_string<CharType>> enum_keys(const std::basic_string<CharType> &section) const;
  std::vector<std::pair<std::basic_string<CharType>, std::basic_string<CharType>>>
  enum_key_values(const std::basic_string<CharType> &section) const;
  bool has_key(const std::basic_string<CharType> &section, const std::basic_string<CharType> &key) const;
  std::basic_string<CharType> get_value(const std::basic_string<CharType> &section,
                                        const std::basic_string<CharType> &key) const;
  bool set_value(const std::basic_string<CharType> &section,
                 const std::basic_string<CharType> &key,
                 const std::basic_string<CharType> &value,
                 const std::basic_string<CharType> &comment = {});
  bool remove_value(const std::basic_string<CharType> &section, const std::basic_string<CharType> &key);

private:
  ini_data data_;
};

template <typename CharType>
inline bool ini_t<CharType>::load(const TCHAR *path) {
  std::basic_string<CharType> content = file::read_text_auto(path);
  if (content.empty()) {
    return false;
  }
  return parse(content);
}

template <>
inline bool ini_t<wchar_t>::load(const TCHAR *path) {
  std::wstring content = xl::encoding::utf8_to_utf16(file::read_text_auto(path));
  if (content.empty()) {
    return false;
  }
  return parse(content);
}

template <typename CharType>
inline bool ini_t<CharType>::save(const TCHAR *path) const {
  std::basic_string<CharType> content = dump();
  return file::write(path, content);
}

template <>
inline bool ini_t<wchar_t>::save(const TCHAR *path) const {
  std::wstring content = dump();
  return file::write_text_utf16_le(path, content);
}

namespace {

/**
 * file: section+
 * section: section_header? line*
 * section_header: [section_name] comment?
 * line: kv? comment?
 * kv: key? (=value)?
 * section_name: [^\[\]]+
 * key: [^=;]+
 * value: [^;]+
 */

template <typename CharType>
std::basic_string<CharType> escape(const std::basic_string<CharType> &content) {
  if (content.empty()) {
    return content;
  }
  if (content.front() != ' ' && content.front() != '\t' && content.back() != ' ' && content.back() != '\t') {
    return content;
  }
  std::basic_string<CharType> escaped;
  escaped.reserve(content.length() + 2);
  escaped.push_back('"');
  for (size_t i = 0; i < content.length(); ++i) {
    if (content[i] == '\\' || content[i] == '"') {
      escaped.push_back('\\');
    }
    escaped.push_back(content[i]);
  }
  escaped.push_back('"');
  return escaped;
}

template <typename CharType>
std::basic_string<CharType> unescape(const std::basic_string<CharType> &content) {
  if (content.length() < 2 || content.front() != '"' || content.back() != '"') {
    return content;
  }
  std::basic_string<CharType> unescaped;
  unescaped.reserve(content.length());
  for (size_t i = 1; i < content.length() - 1; ++i) {
    if (content[i] == '\\' && i + 1 < content.length() - 1) {
      ++i;
      unescaped.push_back(content[i]);
    } else {
      unescaped.push_back(content[i]);
    }
  }
  return unescaped;
}

template <typename CharType>
void pass_blank(const std::basic_string<CharType> &content, size_t &pos, bool back = false) {
  if ((!back && pos >= content.length()) || (back && pos == 0)) {
    return;
  }
  while (true) {
    switch (content[pos]) {
    case ' ':
    case '\t':
      back ? --pos : ++pos;
      break;
    default:
      return;
    }
  }
}

template <typename CharType>
bool match_key(const std::basic_string<CharType> &content, size_t &pos, std::basic_string<CharType> &key) {
  pass_blank(content, pos);
  size_t begin = pos;
  bool stop = false;
  if (pos < content.length() && content[pos] == '[') {
    return false;
  }
  while (pos < content.length() && !stop) {
    switch (content[pos]) {
    case '\r':
    case '\n':
    case '=':
    case ';':
      stop = true;
      break;
    default:
      ++pos;
      break;
    }
  }
  size_t end = pos;
  if (end != begin && end != 0) {
    --end;
    pass_blank(content, end, true);
    ++end;
  }
  key = content.substr(begin, end - begin);
  key = unescape(key);
  return true;
}

template <typename CharType>
bool match_value(const std::basic_string<CharType> &content, size_t &pos, std::basic_string<CharType> &value) {
  pass_blank(content, pos);
  size_t begin = pos;
  bool stop = false;
  while (pos < content.length() && !stop) {
    switch (content[pos]) {
    case '\r':
    case '\n':
    case ';':
      stop = true;
      break;
    default:
      ++pos;
      break;
    }
  }
  size_t end = pos;
  if (end != begin && end != 0) {
    --end;
    pass_blank(content, end, true);
    ++end;
  }
  value = content.substr(begin, end - begin);
  value = unescape(value);
  return true;
}

template <typename CharType>
bool match_comment(const std::basic_string<CharType> &content, size_t &pos, std::basic_string<CharType> &comment) {
  pass_blank(content, pos);
  if (pos >= content.length() || content[pos] != ';') {
    return true;
  }
  ++pos;
  size_t begin = pos;
  while (pos < content.length() && (content[pos] != '\r' && content[pos] != '\n')) {
    ++pos;
  }
  comment = content.substr(begin, pos - begin);
  return true;
}

template <typename CharType>
bool match_line_ending(const std::basic_string<CharType> &content, size_t &pos) {
  while (content[pos] == '\r' && pos < content.length()) {
    ++pos;
  }
  if (pos >= content.length()) {
    return true;
  }
  if (content[pos] != '\n') {
    return false;
  }
  ++pos;
  return true;
}

template <typename CharType>
bool match_line(const std::basic_string<CharType> &content, size_t &pos, typename ini_t<CharType>::ini_line &line) {
  if (!match_key(content, pos, line.key)) {
    return false;
  }
  if (pos < content.length() && content[pos] == '=') {
    ++pos;
    if (!match_value(content, pos, line.value)) {
      return false;
    }
  }
  if (!match_comment(content, pos, line.comment)) {
    return false;
  }
  if (!match_line_ending(content, pos)) {
    return false;
  }
  return true;
}

template <typename CharType>
bool match_section_name(const std::basic_string<CharType> &content, size_t &pos, std::basic_string<CharType> &name) {
  pass_blank(content, pos);
  size_t begin = pos;
  while (pos < content.length() && content[pos] != ']') {
    ++pos;
  }
  if (pos >= content.length()) {
    return false;
  }
  size_t end = pos;
  if (end != begin && end != 0) {
    --end;
    pass_blank(content, end, true);
    ++end;
  }
  name = content.substr(begin, end - begin);
  return true;
}

template <typename CharType>
bool match_section_header(const std::basic_string<CharType> &content,
                          size_t &pos,
                          std::basic_string<CharType> &name,
                          std::basic_string<CharType> &comment) {
  pass_blank(content, pos);
  if (pos < content.length() && content[pos] != '[') {
    return true;
  }
  ++pos;
  if (!match_section_name(content, pos, name)) {
    return false;
  }
  if (pos >= content.length() || content[pos] != ']') {
    return false;
  }
  ++pos;
  if (!match_comment(content, pos, comment)) {
    return false;
  }
  if (!match_line_ending(content, pos)) {
    return false;
  }
  return true;
}

template <typename CharType>
void section_add_line(typename ini_t<CharType>::ini_section &section, typename ini_t<CharType>::ini_line line) {
  auto it = section.line_map.find(line.key);
  if (it != section.line_map.end()) {
    section.lines.erase(it->second);
  }
  section.lines.push_back(std::move(line));
  section.line_map[section.lines.back().key] = std::prev(section.lines.end());
}

template <typename CharType>
bool match_section(const std::basic_string<CharType> &content,
                   size_t &pos,
                   typename ini_t<CharType>::ini_section &section) {
  if (!match_section_header(content, pos, section.name, section.comment)) {
    return false;
  }
  while (true) {
    typename ini_t<CharType>::ini_line line;
    if (!match_line(content, pos, line)) {
      break;
    }
    if (!line.key.empty() || !line.value.empty() || !line.comment.empty()) {
      section_add_line<CharType>(section, std::move(line));
    }
    if (pos >= content.length()) {
      break;
    }
  }
  return true;
}

template <typename CharType>
bool match_file(const std::basic_string<CharType> &content, size_t &pos, typename ini_t<CharType>::ini_data &data) {
  while (pos < content.size()) {
    typename ini_t<CharType>::ini_section section;
    if (!match_section(content, pos, section)) {
      return false;
    }
    auto it = data.section_map.find(section.name);
    if (it != data.section_map.end()) {
      if (!section.comment.empty()) {
        it->second->comment = section.comment;
        for (auto &line : section.lines) {
          section_add_line<CharType>(*it->second, std::move(line));
        }
      }
    } else {
      data.sections.push_back(std::move(section));
      data.section_map[data.sections.back().name] = std::prev(data.sections.end());
    }
  }
  return true;
}

} // namespace

template <typename CharType>
bool ini_t<CharType>::parse(const std::basic_string<CharType> &content) {
  typename ini_t<CharType>::ini_data data;
  size_t pos = 0;
  if (!match_file(content, pos, data)) {
    return false;
  }
  data_ = std::move(data);
  return true;
}

namespace {

template <typename CharType>
size_t calc_aligment(const std::list<typename ini_t<CharType>::ini_line> &lines,
                     typename std::list<typename ini_t<CharType>::ini_line>::const_iterator &it) {
  size_t alignment = 0;
  while (it != lines.end()) {
    if ((it->key.empty() && it->value.empty()) || it->comment.empty()) {
      ++it;
      break;
    }
    alignment = std::max(alignment, escape(it->key).length() + 3 + escape(it->value).length());
    ++it;
  }
  return alignment;
}

template <typename CharType>
std::basic_string<CharType> dump_alignment(size_t used_size, size_t alignment) {
  return std::basic_string<CharType>((alignment > 0 ? alignment - used_size : 0) + (used_size > 0 ? 1 : 0), ' ');
}

template <typename CharType>
std::basic_string<CharType>
dump_comment(const std::basic_string<CharType> &comment, size_t used_size, size_t alignment = 0) {
  if (comment.empty()) {
    return std::basic_string<CharType>();
  }
  std::basic_stringstream<CharType> ss;
  ss << dump_alignment<CharType>(used_size, alignment) << ';' << comment;
  return ss.str();
}

template <typename CharType>
std::basic_string<CharType> dump_line(const typename ini_t<CharType>::ini_line &line, size_t alignment = 0) {
  std::basic_stringstream<CharType> ss;
  if (!line.key.empty() || !line.value.empty()) {
    std::basic_string<CharType> key = escape(line.key), value = escape(line.value);
    ss << key << ' ' << '=' << ' ' << value
       << dump_comment<CharType>(line.comment, key.length() + 3 + value.length(), alignment);
  } else if (!line.comment.empty()) {
    ss << dump_comment<CharType>(line.comment, 0, 0);
  }
  ss << '\r' << '\n';
  return ss.str();
}

template <typename CharType>
std::basic_string<CharType> dump_section_header(const std::basic_string<CharType> &name,
                                                const std::basic_string<CharType> &comment,
                                                size_t alignment = 0) {
  std::basic_stringstream<CharType> ss;
  if (!name.empty() || !comment.empty()) {
    ss << '[' << name << ']' << dump_comment(comment, name.length() + 2, alignment) << '\r' << '\n';
  }
  return ss.str();
}

template <typename CharType>
std::basic_string<CharType> dump_section(const typename ini_t<CharType>::ini_section &section) {
  std::basic_stringstream<CharType> ss;
  auto it_alignment = section.lines.begin();
  size_t alignment = calc_aligment<CharType>(section.lines, it_alignment);
  alignment = std::max(alignment, section.name.length() + 2);
  ss << dump_section_header<CharType>(section.name, section.comment, alignment);
  for (auto it = section.lines.begin(); it != section.lines.end(); ++it) {
    if (it == it_alignment) {
      alignment = calc_aligment<CharType>(section.lines, it_alignment);
    }
    ss << dump_line<CharType>(*it, alignment);
  }
  return ss.str();
}

template <typename CharType>
std::basic_string<CharType> dump_file(const typename ini_t<CharType>::ini_data &data) {
  std::basic_stringstream<CharType> ss;
  for (const auto &section : data.sections) {
    ss << dump_section<CharType>(section) << '\r' << '\n';
  }
  return ss.str();
}

} // namespace

template <typename CharType>
std::basic_string<CharType> ini_t<CharType>::dump() const {
  return dump_file<CharType>(data_);
}

template <typename CharType>
const typename ini_t<CharType>::ini_data &ini_t<CharType>::data() const {
  return data_;
}

template <typename CharType>
std::vector<std::basic_string<CharType>> ini_t<CharType>::enum_sections() const {
  std::vector<std::basic_string<CharType>> section_names;
  section_names.reserve(data_.sections.size());
  for (const auto &section : data_.sections) {
    section_names.push_back(section.name);
  }
  return section_names;
}

template <typename CharType>
bool ini_t<CharType>::has_section(const std::basic_string<CharType> &section) const {
  return data_.section_map.find(section) != data_.section_map.end();
}

template <typename CharType>
bool ini_t<CharType>::add_section(const std::basic_string<CharType> &section,
                                  const std::basic_string<CharType> &comment) {
  auto it = data_.section_map.find(section);
  if (it == data_.section_map.end()) {
    data_.sections.push_back({section, comment, {}, {}});
    data_.section_map[section] = std::prev(data_.sections.end());
  } else {
    if (it->second->comment == comment) {
      return false;
    }
    it->second->comment = comment;
  }
  return true;
}

template <typename CharType>
bool ini_t<CharType>::remove_section(const std::basic_string<CharType> &section) {
  auto it = data_.section_map.find(section);
  if (it == data_.section_map.end()) {
    return false;
  }
  data_.sections.erase(it->second);
  data_.section_map.erase(it);
  return true;
}

template <typename CharType>
std::vector<std::basic_string<CharType>> ini_t<CharType>::enum_keys(const std::basic_string<CharType> &section) const {
  auto it = data_.section_map.find(section);
  if (it == data_.section_map.end()) {
    return {};
  }
  std::vector<std::basic_string<CharType>> keys;
  for (const auto &line : it->second->lines) {
    keys.push_back(line.key);
  }
  return keys;
}

template <typename CharType>
std::vector<std::pair<std::basic_string<CharType>, std::basic_string<CharType>>>
ini_t<CharType>::enum_key_values(const std::basic_string<CharType> &section) const {
  auto it = data_.section_map.find(section);
  if (it == data_.section_map.end()) {
    return {};
  }
  std::vector<std::pair<std::basic_string<CharType>, std::basic_string<CharType>>> kv_list;
  for (const auto &line : it->second->lines) {
    kv_list.push_back({line.key, line.value});
  }
  return kv_list;
}

template <typename CharType>
bool ini_t<CharType>::has_key(const std::basic_string<CharType> &section,
                              const std::basic_string<CharType> &key) const {
  auto it_section = data_.section_map.find(section);
  if (it_section == data_.section_map.end()) {
    return false;
  }
  const auto &line_map = it_section->second->line_map;
  auto it_key = line_map.find(key);
  if (it_key == line_map.end()) {
    return false;
  }
  return true;
}

template <typename CharType>
std::basic_string<CharType> ini_t<CharType>::get_value(const std::basic_string<CharType> &section,
                                                       const std::basic_string<CharType> &key) const {
  auto it_section = data_.section_map.find(section);
  if (it_section == data_.section_map.end()) {
    return std::basic_string<CharType>();
  }
  const auto &line_map = it_section->second->line_map;
  auto it_key = line_map.find(key);
  if (it_key == line_map.end()) {
    return std::basic_string<CharType>();
  }
  return it_key->second->value;
}

template <typename CharType>
bool ini_t<CharType>::set_value(const std::basic_string<CharType> &section,
                                const std::basic_string<CharType> &key,
                                const std::basic_string<CharType> &value,
                                const std::basic_string<CharType> &comment) {
  auto it_section = data_.section_map.find(section);
  if (it_section == data_.section_map.end()) {
    data_.sections.push_back({section, comment, {}, {}});
    data_.section_map[section] = std::prev(data_.sections.end());
    it_section = data_.section_map.find(section);
  }
  auto &ini_section = *it_section->second;
  auto it_key = ini_section.line_map.find(key);
  if (it_key == ini_section.line_map.end()) {
    ini_section.lines.push_back({key, value, comment});
    ini_section.line_map[key] = std::prev(ini_section.lines.end());
  } else {
    if (it_key->second->value == value && it_key->second->comment == comment) {
      return false;
    }
    it_key->second->value = value;
    it_key->second->comment = comment;
  }
  return true;
}

template <typename CharType>
bool ini_t<CharType>::remove_value(const std::basic_string<CharType> &section, const std::basic_string<CharType> &key) {
  auto it_section = data_.section_map.find(section);
  if (it_section == data_.section_map.end()) {
    return false;
  }
  auto &ini_section = *it_section->second;
  auto it_key = ini_section.line_map.find(key);
  if (it_key == ini_section.line_map.end()) {
    return false;
  }
  ini_section.lines.erase(it_key->second);
  ini_section.line_map.erase(it_key);
  return false;
}

} // namespace xl
