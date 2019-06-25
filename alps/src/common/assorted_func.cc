/* 
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "alps/common/assorted_func.hh"

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace alps {

std::string os_error() {
  return os_error(errno);
}

std::string os_error(int error_number) {
  if (error_number == 0) {
    return "[No Error]";
  }
  std::stringstream str;
  str << "[Errno " << error_number << "] " << std::strerror(error_number);
  return str.str();
}

std::string demangle_type_name(const char* mangled_name) {
#ifdef __GNUC__
  int status;
  char* demangled = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
  if (demangled) {
    std::string ret(demangled);
    ::free(demangled);
    return ret;
  }
#endif  // __GNUC__
  return mangled_name;
}

std::ostream& operator<<(std::ostream& o, const Hex& v) {
    std::ios::fmtflags old_flags = o.flags();
    o << "0x";
    if (v.fix_digits_ >= 0) {
        o.width(v.fix_digits_);
        o.fill('0');
    }
    o << std::hex << std::uppercase << v.val_;
    o.flags(old_flags);
    return o;
}

std::ostream& operator<<(std::ostream& o, const HexString& v) {
    std::ios::fmtflags old_flags = o.flags();
    o << "0x";
    o.width(2);
    o.fill('0');
    o << std::hex << std::uppercase;
    for (uint32_t i = 0; i < v.str_.size() && i < v.max_bytes_; ++i) {
        if (i > 0 && i % 8U == 0) {
            o << " ";  // put space for every 8 bytes for readability
        }
        o << static_cast<uint16_t>(v.str_[i]);
    }
    o.flags(old_flags);
    if (v.max_bytes_ != -1U && v.str_.size() > v.max_bytes_) {
        o << " ...(" << (v.str_.size() - v.max_bytes_) << " more bytes)";
    }
    return o;
}

/**
 * @brief Convert a string that contains units to an integer.
 * @ingroup ASSORTED
 */

uint64_t string_to_size(std::string str)
{
    char     unit = str.back();
    uint64_t mult = 1;
    
    switch (unit) {
        case 'K': case 'k':
            mult = 1024LLU;
            str.pop_back();
            break;

        case 'M': case 'm':
            mult = 1024LLU * 1024LLU;
            str.pop_back();
            break;

        case 'G': case 'g':
            mult = 1024LLU * 1024LLU * 1024LLU;
            str.pop_back();
            break;
    }

    return mult * stoull(str);
}

std::vector<std::string> split_string(std::string s, std::string delimiter)
{
    size_t pos;
    std::vector<std::string> sl;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        std::string token = s.substr(0, pos);
        sl.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    if (s.length()) {
        sl.push_back(s);
    }
    return sl;
}

std::list<size_t> expand_range_token(std::string range_token, std::string delimiter)
{
    std::list<size_t> range_list;
    int pos = range_token.find(delimiter);

    if (pos < 0) {
        range_list.push_back(stoi(range_token));
    } else {
        size_t low = stoi(range_token.substr(0, pos));
        size_t high = stoi(range_token.substr(pos+1, range_token.length()));
        for (size_t id = low; id <= high; id++) {
            range_list.push_back(id);
        }
    }

    return range_list;
} 

std::list<size_t> expand_range(std::string range)
{
    std::list<size_t> range_list;

    std::vector<std::string> tokens = split_string(range, ",");
    for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++) 
    {
        range_list.merge(expand_range_token(*it, "-"));
    }
    return range_list;
}

std::string longest_common_prefix(std::vector<std::string>& sv)
{
    std::string result;
    
    if (sv.size() > 0) {
        result = sv[0];
    }
    
    //in each iteration try to match the whole prefix; if not matching reduce it
    for (size_t i=1; i<sv.size(); i++)
    {
        size_t m = std::min(result.size(), sv[i].size());
        size_t j;
        for (j=0; j<m; j++)
        {
            if (result[j] != sv[i][j]) {
                break;
            }
        }
        result = result.substr(0,j);
        //this condition will prevent further checking of the prefix is already empty
        if (result.empty()) {
            break;
        }
    }
    return result;
}


} // namespace alps
