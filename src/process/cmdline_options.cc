// MIT License
//
// Copyright (c) 2024 Streamlet (streamlet@outlook.com)
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

#include <cstring>
#include <cwchar>
#include <vector>
#include <xl/cmdline_options>
#include <xl/native_string>
#include <xl/scope_exit>
#ifdef _WIN32
#include <xl/encoding>
// clang-format off
// Windows.h MUST be included before ShellAPI.h
#include <Windows.h>
// clang-format on
#include <ShellAPI.h>
#endif

namespace xl {

namespace cmdline_options {

namespace {

parsed_options parse_native(int argc, const TCHAR *argv[]) {
  std::map<native_string, native_string> result;
  for (int i = 0; i < argc; ++i) {
    native_string arg = argv[i];
    if (arg.length() >= 2 && arg[0] == _T('-') && arg[1] == _T('-')) {
      auto equal_pos = arg.find_first_of(_T('='), 2);
      if (equal_pos != native_string::npos) {
        native_string k = arg.substr(2, equal_pos - 2);
        native_string v = arg.substr(equal_pos + 1);
        result.insert(std::make_pair(k, v));
      } else {
        native_string k = arg.substr(2);
        if (i + 1 < argc && argv[i + 1][0] != _T('-')) {
          result.insert(std::make_pair(k, argv[i + 1]));
          ++i;
        } else {
          result.insert(std::make_pair(k, native_string()));
        }
      }
    } else if (arg.length() >= 1 && arg[0] == _T('-')) {
      native_string k = arg.substr(1);
      if (i + 1 < argc && argv[i + 1][0] != _T('-')) {
        result.insert(std::make_pair(k, argv[i + 1]));
        ++i;
      } else {
        result.insert(std::make_pair(k, native_string()));
      }
    } else {
      result.insert(std::make_pair(arg, native_string()));
    }
  }
  return {std::move(result)};
}

} // namespace

parsed_options parse(int argc, const TCHAR *argv[]) {
  return parse_native(argc - 1, &argv[1]);
}

#ifdef _WIN32

#ifdef _UNICODE

parsed_options parse(const wchar_t *cmdline) {
  int argc = 0;
  LPWSTR *argv = ::CommandLineToArgvW(cmdline, &argc);
  if (argv == NULL) {
    return {};
  }
  XL_ON_BLOCK_EXIT(::LocalFree, argv);
  return parse_native(argc, (LPCWSTR *)argv);
}

#else

parsed_options parse(const char *cmdline) {
  std::wstring wstr = encoding::ansi_to_utf16(cmdline);
  int argc = 0;
  LPWSTR *argv = ::CommandLineToArgvW(wstr.c_str(), &argc);
  if (argv == NULL) {
    return {};
  }
  XL_ON_BLOCK_EXIT(::LocalFree, argv);
  std::vector<std::string> argv_str;
  for (int i = 0; i < argc; ++i) {
    argv_str.push_back(encoding::utf16_to_ansi(argv[i]));
  }
  std::vector<const char *> argv_cstr;
  for (int i = 0; i < argc; ++i) {
    argv_cstr.push_back(argv_str[i].c_str());
  }
  return parse_native(argc, &argv_cstr[0]);
}

#endif

#endif

} // namespace cmdline_options

} // namespace xl
