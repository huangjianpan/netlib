/**
 * @author huangjianpan
 * C++17
 */
#pragma once

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace json {

class Json {
 private:
  static constexpr const char* ErrMsg[] = {
      "build error: format error",                           // 0
      "build object error: mismatch of right '\"' of name",  // 1
      "build object error: mismatch ':'",                    // 2
      "build object error: no memory",                       // 3
      "build object error: mismatch ',' or '}'",             // 4
      "build object error: mismatch of left '\"' of name",   // 5
      "build bool error",                                    // 6
      "build numeric error",                                 // 7
      "build string error: mismatch of right '\"'",          // 8
      "build string error: no memory",                       // 9
      "build array error: no memory",                        // 10
      "build array error: mismatch ',' or ']'"               // 11
  };

  using ObjectData = std::unordered_map<std::string, Json>;
  using ArrayData = std::vector<Json>;
  using StringData = std::string;

 public:
  struct Type {
    static constexpr size_t Null = 1 << 0;
    static constexpr size_t Bool = 1 << 1;
    static constexpr size_t Integer = 1 << 2;
    static constexpr size_t Float = 1 << 3;
    static constexpr size_t Error = 1 << 4;
    static constexpr size_t String = 1 << 5;
    static constexpr size_t Object = 1 << 6;
    static constexpr size_t Array = 1 << 7;
  };

  Json() : type_(Type::Object), data_(nullptr) {}

  Json(Json&& rhs);

  Json(const Json& rhs) = delete;

  // use is_error() or errmsg() to check whether it was failed to deserialize.
  Json(const char* raw) { build(raw); }

  // use is_error() or errmsg() to check whether it was failed to deserialize.
  Json(const std::string& raw) { build(raw.c_str()); }

  ~Json() { free(); }

  Json& operator=(Json&& rhs);

  Json& operator=(const Json& rhs) = delete;

  Json& operator[](size_t i) const;

  template <typename T>
  Json& operator[](T i) const {
    static_assert(std::is_integral<T>::value, "Type T should be integer");
    return operator[](static_cast<size_t>(i));
  }

  Json& operator[](const std::string& key) const;

  Json& operator[](const char* key) const {
    return operator[](std::string(key));
  }

  bool emplace(std::string key, Json&& value);

  // TODO: const char(&)[5] -> const char*
  template <typename Key, typename Value>
  bool emplace(Key&& key, Value&& value) {
    using R_Key = typename std::remove_reference<Key>::type;
    using R_Value = typename std::remove_reference<Value>::type;

    static_assert(std::is_same<R_Key, std::string>::value ||
                      std::is_same<R_Key, const char*>::value,
                  "Type Key should be std::string or const char*");
    if constexpr (std::is_same<R_Value, bool>::value) {
      return emplace(std::forward<Key>(key), Json((bool)value));
    }
    if constexpr (std::is_integral<R_Value>::value) {
      return emplace(std::forward<Key>(key), Json((long long)value));
    }
    if constexpr (std::is_floating_point<R_Value>::value) {
      return emplace(std::forward<Key>(key), Json((double)value));
    }
    if constexpr (std::is_same<R_Value, const char*>::value ||
                  std::is_same<R_Value, std::string>::value) {
      return emplace(
          std::forward<Key>(key),
          Json(Type::String, new std::string(std::forward<Value>(value))));
    }
    if constexpr (std::is_same<R_Value, Json>::value) {
      return emplace(std::forward<Key>(key), std::forward<Value>(value));
    }
    return false;
  }

  template <typename T>
  operator T() const {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value ||
            std::is_same<T, std::string>::value || std::is_same<T, bool>::value,
        "Type T should be integer or float");
    if constexpr (std::is_same<T, bool>::value) {  // note: bool is integral
      return is_bool() ? b_ : false;
    }
    if constexpr (std::is_integral<T>::value) {
      return is_integer() ? static_cast<T>(i_)
                          : (is_float() ? static_cast<T>(f_) : 0);
    }
    if constexpr (std::is_floating_point<T>::value) {
      return is_float() ? static_cast<T>(f_)
                        : (is_integer() ? static_cast<T>(i_) : 0);
    }
    if constexpr (std::is_same<T, std::string>::value) {
      return (is_string() && data_ != nullptr)
                 ? *static_cast<std::string*>(data_)
                 : "";
    }
  }

  // errmsg: output error message.
  const char* errmsg() const {
    return is_error() ? static_cast<const char*>(data_) : nullptr;
  }

  // marshal: serialize the json object.
  // judge whether the type is object before invoking this function.
  std::string marshal() const {
    std::string ret(marshal_count(), ' ');
    char* p = marshal(&ret[0]);
    ret.resize(p - &ret[0]);
    return ret;
  }

  // marshal: serialize the json object to the buffer.
  // judge whether the type is object before invoking this function.
  // @return point to the next character after serialized the json object.
  char* marshal(char* buffer) const;

  // marshal_count: calculate the approximate length of serizalization.
  size_t marshal_count() const;

  bool is_null() const { return type_ & Type::Null; }

  bool is_bool() const { return type_ & Type::Bool; }

  bool is_numeric() const { return type_ & (Type::Integer | Type::Float); }

  bool is_error() const { return type_ & Type::Error; }

  bool is_object() const { return type_ & Type::Object; }

  bool is_array() const { return type_ & Type::Array; }

  bool is_string() const { return type_ & Type::String; }

 private:
  bool is_integer() const { return type_ & Type::Integer; }

  bool is_float() const { return type_ & Type::Float; }

  bool is_use_data() const {
    return type_ & (Type::String | Type::Object | Type::Array | Type::Error);
  }

  void free() {
    if (is_string()) {
      delete static_cast<std::string*>(data_);
    } else if (is_object()) {
      delete static_cast<std::unordered_map<std::string, Json>*>(data_);
    } else {
      delete static_cast<std::vector<Json>*>(data_);
    }
  }

  Json(bool b) : type_(Type::Bool), b_(b) {}

  Json(long long i) : type_(Type::Integer), i_(i) {}

  Json(double f) : type_(Type::Float), f_(f) {}

  Json(size_t type, void* data) : type_(type), data_(data) {}

  void build(const char* raw);

  static Json build(const char* raw, const char*& next);

  // build_object: *raw is '{' and *(next - 1) is '}'
  static Json build_object(const char* raw, const char*& next);

  static Json build_bool(const char* raw, const char*& next);

  static Json build_numeric(const char* raw, const char*& next);

  static Json build_string(const char* raw, const char*& next);

  static Json build_array(const char* raw, const char*& next);

  static Json build_error(const char* errmsg) {
    return Json(Type::Error, (void*)errmsg);
  }

 private:
  size_t type_;
  union {
    bool b_;
    long long i_;
    double f_;

    // data_: the underlaying data of string, object, array
    //   string is std::string*
    //   object is std::unordered_map<std::string, Json>*
    //   array  is std::vector<Json>*
    void* data_;
  };

  static Json g_null_json_;
};

namespace tool {

inline const char* skip_blank(const char* p) {
  while (*p == ' ' || *p == '\n' || *p == '\t') {
    ++p;
  }
  return p;
}

inline const char* find(const char* p, char t) {
  while (*p != '\0' && *p != t) {
    ++p;
  }
  return p;
}

}  // namespace tool

}  // namespace json
