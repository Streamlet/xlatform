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

#include "ini.h"
#include <algorithm>
#include <sstream>
#include <xl/file>
#include <xl/ini>

namespace xl {

namespace ini {

std::vector<std::string> enum_sections(const TCHAR *path) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_sections();
}
std::vector<std::wstring> enum_sections_w(const TCHAR *path) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_sections();
}

bool has_section(const TCHAR *path, const std::string &section) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return false;
  }
  return ini.has_section(section);
}
bool has_section(const TCHAR *path, const std::wstring &section) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return false;
  }
  return ini.has_section(section);
}

bool add_section(const TCHAR *path, const std::string &section, const std::string &comment) {
  xl::ini_t<char> ini;
  ini.load(path);
  if (!ini.add_section(section, comment)) {
    return false;
  }
  return ini.save(path);
}
bool add_section(const TCHAR *path, const std::wstring &section, const std::wstring &comment) {
  xl::ini_t<wchar_t> ini;
  ini.load(path);
  if (!ini.add_section(section, comment)) {
    return false;
  }
  return ini.save(path);
}

bool remove_section(const TCHAR *path, const std::string &section) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return false;
  }
  if (!ini.remove_section(section)) {
    return false;
  }
  return ini.save(path);
}
bool remove_section(const TCHAR *path, const std::wstring &section) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return false;
  }
  if (!ini.remove_section(section)) {
    return false;
  }
  return ini.save(path);
}

std::vector<std::string> enum_keys(const TCHAR *path, const std::string &section) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_keys(section);
}
std::vector<std::wstring> enum_keys(const TCHAR *path, const std::wstring &section) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_keys(section);
}

std::vector<std::pair<std::string, std::string>> enum_key_values(const TCHAR *path, const std::string &section) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_key_values(section);
}
std::vector<std::pair<std::wstring, std::wstring>> enum_key_values(const TCHAR *path, const std::wstring &section) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.enum_key_values(section);
}

bool has_key(const TCHAR *path, const std::string &section, const std::string &key) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.has_key(section, key);
}
bool has_key(const TCHAR *path, const std::wstring &section, const std::wstring &key) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.has_key(section, key);
}

std::string get_value(const TCHAR *path, const std::string &section, const std::string &key) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.get_value(section, key);
}
std::wstring get_value(const TCHAR *path, const std::wstring &section, const std::wstring &key) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return {};
  }
  return ini.get_value(section, key);
}
bool set_value(const TCHAR *path,
               const std::string &section,
               const std::string &key,
               const std::string &value,
               const std::string &comment) {
  xl::ini_t<char> ini;
  ini.load(path);
  if (!ini.set_value(section, key, value, comment)) {
    return false;
  }
  return ini.save(path);
}
bool set_value(const TCHAR *path,
               const std::wstring &section,
               const std::wstring &key,
               const std::wstring &value,
               const std::wstring &comment) {
  xl::ini_t<wchar_t> ini;
  ini.load(path);
  if (!ini.set_value(section, key, value, comment)) {
    return false;
  }
  return ini.save(path);
}

bool remove_value(const TCHAR *path, const std::string &section, const std::string &key) {
  xl::ini_t<char> ini;
  if (!ini.load(path)) {
    return false;
  }
  if (!ini.remove_value(section, key)) {
    return false;
  }
  return ini.save(path);
}
bool remove_value(const TCHAR *path, const std::wstring &section, const std::wstring &key) {
  xl::ini_t<wchar_t> ini;
  if (!ini.load(path)) {
    return false;
  }
  if (!ini.remove_value(section, key)) {
    return false;
  }
  return ini.save(path);
}

} // namespace ini

} // namespace xl
