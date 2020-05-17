#ifndef __OOKNN_REFLECT_HPP__
#define __OOKNN_REFLECT_HPP__

#include <rapidjson/document.h>
#include <rapidjson/fwd.h>
#include <rapidjson/prettywriter.h>

#include <boost/preprocessor/seq/for_each.hpp>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ooknn {

struct JsonNull {};

struct JsonReader {
  rapidjson::Value *m;
  std::vector<const char *> path_;

  JsonReader(rapidjson::Value *m) : m(m) {}
  void startObject() {}
  void endObject() {}
  void iterArray(const std::function<void()> &fn);
  void member(const char *name, const std::function<void()> &fn);
  bool isNull();
  std::string getString();
  std::string getPath() const;
};

struct JsonWriter {
  using W =
      rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<char>,
                        rapidjson::UTF8<char>, rapidjson::CrtAllocator, 0>;

  W *m;

  JsonWriter(W *m) : m(m) {}
  void startArray();
  void endArray();
  void startObject();
  void endObject();
  void key(const char *name);
  void null_();
  void int64(int64_t v);
  void string(const char *s);
  void string(const char *s, size_t len);
};

// clang-format off

std::string JsonReader::getString() { return m->GetString(); }
bool JsonReader::isNull() { return m->IsNull(); }

void JsonWriter::startArray() { m->StartArray(); }
void JsonWriter::endArray() { m->EndArray(); }
void JsonWriter::startObject() { m->StartObject(); }
void JsonWriter::endObject() { m->EndObject(); }
void JsonWriter::key(const char* name) { m->Key(name); }
void JsonWriter::null_() { m->Null(); }
void JsonWriter::int64(int64_t v) { m->Int64(v); }
void JsonWriter::string(const char* s) { m->String(s); }
void JsonWriter::string(const char* s, size_t len) { m->String(s, len); }

void reflect(JsonReader &vis, bool &v              ) { if (!vis.m->IsBool())   throw std::invalid_argument("bool");               v = vis.m->GetBool(); }
void reflect(JsonReader &vis, unsigned char &v     ) { if (!vis.m->IsInt())    throw std::invalid_argument("uint8_t");            v = (uint8_t)vis.m->GetInt(); }
void reflect(JsonReader &vis, short &v             ) { if (!vis.m->IsInt())    throw std::invalid_argument("short");              v = (short)vis.m->GetInt(); }
void reflect(JsonReader &vis, unsigned short &v    ) { if (!vis.m->IsInt())    throw std::invalid_argument("unsigned short");     v = (unsigned short)vis.m->GetInt(); }
void reflect(JsonReader &vis, int &v               ) { if (!vis.m->IsInt())    throw std::invalid_argument("int");                v = vis.m->GetInt(); }
void reflect(JsonReader &vis, unsigned &v          ) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned");           v = (unsigned)vis.m->GetUint64(); }
void reflect(JsonReader &vis, long &v              ) { if (!vis.m->IsInt64())  throw std::invalid_argument("long");               v = (long)vis.m->GetInt64(); }
void reflect(JsonReader &vis, unsigned long &v     ) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned long");      v = (unsigned long)vis.m->GetUint64(); }
void reflect(JsonReader &vis, long long &v         ) { if (!vis.m->IsInt64())  throw std::invalid_argument("long long");          v = vis.m->GetInt64(); }
void reflect(JsonReader &vis, unsigned long long &v) { if (!vis.m->IsUint64()) throw std::invalid_argument("unsigned long long"); v = vis.m->GetUint64(); }
void reflect(JsonReader &vis, double &v            ) { if (!vis.m->IsDouble()) throw std::invalid_argument("double");             v = vis.m->GetDouble(); }
void reflect(JsonReader &vis, std::string &v       ) { if (!vis.m->IsString()) throw std::invalid_argument("string");             v = vis.getString(); }

void reflect(JsonWriter &vis, bool &v              ) { vis.m->Bool(v); }
void reflect(JsonWriter &vis, unsigned char &v     ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, short &v             ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, unsigned short &v    ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, int &v               ) { vis.m->Int(v); }
void reflect(JsonWriter &vis, unsigned &v          ) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, long &v              ) { vis.m->Int64(v); }
void reflect(JsonWriter &vis, unsigned long &v     ) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, long long &v         ) { vis.m->Int64(v); }
void reflect(JsonWriter &vis, unsigned long long &v) { vis.m->Uint64(v); }
void reflect(JsonWriter &vis, double &v            ) { vis.m->Double(v); }
void reflect(JsonWriter &vis, std::string &v       ) { vis.string(v.c_str(), v.size()); }
void reflect(JsonReader& vis, JsonNull& v) {}
void reflect(JsonWriter& vis, JsonNull& v) { vis.m->Null(); }

// clang-format on

// std::vector
template <typename T>
void reflect(JsonReader &vis, std::vector<T> &v) {
  vis.iterArray([&]() {
    v.emplace_back();
    reflect(vis, v.back());
  });
}
template <typename T>
void reflect(JsonWriter &vis, std::vector<T> &v) {
  vis.startArray();
  for (auto &it : v) reflect(vis, it);
  vis.endArray();
}

void reflectMemberStart(JsonReader &vis) {
  if (!vis.m->IsObject()) throw std::invalid_argument("object");
}

template <typename T>
void reflectMemberStart(T &) {}
inline void reflectMemberStart(JsonWriter &vis) { vis.startObject(); }

template <typename T>
void reflectMemberEnd(T &) {}
inline void reflectMemberEnd(JsonWriter &vis) { vis.endObject(); }

template <typename T>
void reflectMember(JsonReader &vis, const char *name, T &v) {
  vis.member(name, [&]() { reflect(vis, v); });
}
template <typename T>
void reflectMember(JsonWriter &vis, const char *name, T &v) {
  vis.key(name);
  reflect(vis, v);
}

void JsonReader::iterArray(const std::function<void()> &fn) {
  if (!m->IsArray()) throw std::invalid_argument("array");
  path_.push_back("0");
  for (auto &entry : m->GetArray()) {
    auto saved = m;
    m = &entry;
    fn();
    m = saved;
  }
  path_.pop_back();
}
void JsonReader::member(const char *name,const std::function<void()> &fn) {
  path_.push_back(name);
  auto it = m->FindMember(name);
  if (it != m->MemberEnd()) {
    auto saved = m;
    m = &it->value;
    fn();
    m = saved;
  }
  path_.pop_back();
}

#define REFLECT_MEMBER(name) reflectMember(vis, #name, v.name)

#define _MAPPABLE_REFLECT_MEMBER(unuse, type, name) REFLECT_MEMBER(name);

#define REFLECT_STRUCT(type, ...)                                   \
  template <typename Vis>                                           \
  void reflect(Vis &vis, type &v) {                                 \
    reflectMemberStart(vis);                                        \
    BOOST_PP_SEQ_FOR_EACH(_MAPPABLE_REFLECT_MEMBER, _, __VA_ARGS__) \
    reflectMemberEnd(vis);                                          \
  }

}  // namespace ooknn

#endif  // __OOKNN_REFLECT_HPP__
