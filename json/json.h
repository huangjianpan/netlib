// @author huangjianpan
// using C++17

// Note:
// 1. operator[]() possible return Json::g_null_json_ which should not be
// modified, so need to check whether the return value is null.
// 2. use is_null() to judge whether the json object exist.

// Example 1: create object
// json::Json j(json::Json::Object{});
// j.add(k, v)

// Example 2: create array
// json::Json j(json::Json::Array{});
// j.add(elem);

// Example 3: unmarshal and assignment
// const char* errmsg = nullptr;
// json::Json j = json::Json::unmarshal(json_str, errmsg);
// if (errmsg == nullptr) {
//   std::string s = j["names"][0];
//   int age = j["age"]
//   Json& info = j["info"];
//   if (!info.is_null()) {
//     std::cout << (std::string)info << std::endl;
//   }
// } else {
//   std::cout << errmsg << std::endl;
// }

// Example 5: move data
// json::Json j = json::Json::unmarshal(json_str, errmsg);
// std::string name;
// j["name"].move_to(name);

// Example 4: marshal
// json::Json j(json::Json::Object{});
// j.add(k, v);
// ...
// std::cout << j.marshal() << std::endl;

#pragma once

#include <map>
#include <string>
#include <type_traits>
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

  using ObjectData = std::map<std::string, Json>;
  using ArrayData = std::vector<Json>;
  using StringData = std::string;

  template <typename T>
  struct static_array_to_pointer {
    typedef T type;
  };

  template <typename T, size_t N>
  struct static_array_to_pointer<T[N]> {
    typedef T* type;
  };

  struct Type {
    static constexpr size_t Null = 1 << 0;  // only g_null_json_ is null
    static constexpr size_t Bool = 1 << 1;
    static constexpr size_t Integer = 1 << 2;
    static constexpr size_t Float = 1 << 3;
    static constexpr size_t Error = 1 << 4;  // only Json internal use
    static constexpr size_t String = 1 << 5;
    static constexpr size_t Object = 1 << 6;
    static constexpr size_t Array = 1 << 7;
  };

 public:
  struct Object {};

  struct Array {};

  Json(Object) : type_(Type::Object), data_(nullptr) {}

  Json(Array) : type_(Type::Array), data_(nullptr) {}

  Json(Json&& rhs) : type_(rhs.type_) {
    data_ = rhs.data_;    // data_ has the maximum length in the union.
    rhs.data_ = nullptr;  // don't change rhs's type
  }

  Json(const Json& rhs);

  ~Json() { free(); }

  Json& operator=(Json&& rhs);

  Json& operator=(const Json& rhs);

  Json& operator[](size_t i);

  template <typename T>
  Json& operator[](T i) {
    static_assert(std::is_integral<T>::value && !std::is_same<T, bool>::value,
                  "Type T should be integer");
    return operator[](static_cast<size_t>(i));
  }

  Json& operator[](const std::string& key);

  Json& operator[](const char* key) { return operator[](std::string(key)); }

  template <typename Value>
  bool add(std::string key, Value&& value) {
    using R_Value = typename std::remove_reference<Value>::type;
    static_assert(
        std::is_same<R_Value, bool>::value ||
            std::is_integral<R_Value>::value ||
            std::is_floating_point<R_Value>::value ||
            std::is_same<R_Value, const char*>::value ||
            std::is_same<R_Value, std::string>::value ||
            std::is_same<typename static_array_to_pointer<R_Value>::type,
                         const char*>::value ||
            std::is_same<R_Value, Json>::value ||
            std::is_same<R_Value, const Json>::value,
        "Type Value should be bool, integer, float, string, Json");

    if constexpr (std::is_same<R_Value, bool>::value) {
      return add_kv(std::move(key), Json((bool)value));
    } else if constexpr (std::is_integral<R_Value>::value) {
      return add_kv(std::move(key), Json((long long)value));
    } else if constexpr (std::is_floating_point<R_Value>::value) {
      return add_kv(std::move(key), Json((double)value));
    } else if constexpr (std::is_same<R_Value, const char*>::value ||
                         std::is_same<R_Value, std::string>::value ||
                         std::is_same<
                             typename static_array_to_pointer<R_Value>::type,
                             const char*>::value) {
      return add_kv(
          std::move(key),
          Json(Type::String, new std::string(std::forward<Value>(value))));

    } else if constexpr (std::is_same<R_Value, Json>::value ||
                         std::is_same<R_Value, const Json>::value) {
      return add_kv(std::move(key), std::forward<Value>(value));
    } else {
      return false;
    }
  }

  template <typename Value>
  bool add(Value&& value) {
    using R_Value = typename std::remove_reference<Value>::type;
    static_assert(
        std::is_same<R_Value, bool>::value ||
            std::is_integral<R_Value>::value ||
            std::is_floating_point<R_Value>::value ||
            std::is_same<R_Value, const char*>::value ||
            std::is_same<R_Value, std::string>::value ||
            std::is_same<typename static_array_to_pointer<R_Value>::type,
                         const char*>::value ||
            std::is_same<R_Value, Json>::value ||
            std::is_same<R_Value, const Json>::value,
        "Type Value should be bool, integer, float, string, Json");

    if constexpr (std::is_same<R_Value, bool>::value) {
      return add_elem(Json((bool)value));
    } else if constexpr (std::is_integral<R_Value>::value) {
      return add_elem(Json((long long)value));
    } else if constexpr (std::is_floating_point<R_Value>::value) {
      return add_elem(Json((double)value));
    } else if constexpr (std::is_same<R_Value, const char*>::value ||
                         std::is_same<R_Value, std::string>::value ||
                         std::is_same<
                             typename static_array_to_pointer<R_Value>::type,
                             const char*>::value) {
      return add_elem(
          Json(Type::String, new std::string(std::forward<Value>(value))));

    } else if constexpr (std::is_same<R_Value, Json>::value ||
                         std::is_same<R_Value, const Json>::value) {
      return add_elem(std::forward<Value>(value));
    } else {
      return false;
    }
  }

  template <typename T>
  bool move_to(T& ret) {
    static_assert(
        std::is_same<T, bool>::value || std::is_same<T, std::string>::value ||
            std::is_integral<T>::value || std::is_floating_point<T>::value,
        "Type T should be bool, integer, float, string");
    if constexpr (std::is_same<T, bool>::value) {
      ret = is_bool() ? b_ : false;
      return is_bool();
    } else if constexpr (std::is_integral<T>::value) {
      ret = is_integer() ? static_cast<T>(i_) : static_cast<T>(0);
      return is_integer();
    } else if constexpr (std::is_floating_point<T>::value) {
      ret = is_float() ? static_cast<T>(f_) : static_cast<T>(0.0);
      return is_float();
    } else if constexpr (std::is_same<T, std::string>::value) {
      ret = is_string() && data_ != nullptr
                ? std::move(*static_cast<StringData*>(data_))
                : "";
      return is_string();
    } else {
      return false;
    }
  }

  template <typename T>
  bool move_to(std::map<std::string, T>& ret,
               void (*transfer)(Json&, T&) = nullptr) {
    if (is_object()) {
      ret.clear();
      if (data_ == nullptr) {
        return true;
      }
      auto& data = *static_cast<ObjectData*>(data_);
      for (auto& kv : data) {
        if constexpr (std::is_same<T, bool>::value ||
                      std::is_integral<T>::value ||
                      std::is_floating_point<T>::value) {
          ret.emplace(kv.first, static_cast<T>(kv.second));
        } else if constexpr (std::is_same<T, std::string>::value) {
          ret.emplace(
              kv.first,
              kv.second.data_ == nullptr || !kv.second.is_string()
                  ? ""
                  : std::move(*static_cast<StringData*>(kv.second.data_)));
        } else {
          T t;
          transfer(kv.second, t);
          ret.emplace(kv.first, std::move(t));
        }
      }
      return true;
    }
    return false;
  }

  template <typename T>
  bool move_to(std::vector<T>& ret, void (*transfer)(Json&, T&) = nullptr) {
    if (is_array()) {
      ret.clear();
      if (data_ == nullptr) {
        return true;
      }
      auto& data = *static_cast<ArrayData*>(data_);
      ret.reserve(data.size());
      for (auto& e : data) {
        if constexpr (std::is_same<T, bool>::value ||
                      std::is_integral<T>::value ||
                      std::is_floating_point<T>::value) {
          ret.emplace_back(static_cast<T>(e));
        } else if constexpr (std::is_same<T, std::string>::value) {
          ret.emplace_back(e.data_ == nullptr || !e.is_string()
                               ? ""
                               : std::move(*static_cast<StringData*>(e.data_)));
        } else {
          T t;
          transfer(e, t);
          ret.emplace_back(std::move(t));
        }
      }
      return true;
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
    } else if constexpr (std::is_integral<T>::value) {
      return is_integer() ? static_cast<T>(i_)
                          : (is_float() ? static_cast<T>(f_) : 0);
    } else if constexpr (std::is_floating_point<T>::value) {
      return is_float() ? static_cast<T>(f_)
                        : (is_integer() ? static_cast<T>(i_) : 0);
    } else if constexpr (std::is_same<T, std::string>::value) {
      return (is_string() && data_ != nullptr)
                 ? *static_cast<std::string*>(data_)
                 : "";
    }
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

  static Json unmarshal(const char* raw, const char*& errmsg) {
    Json j(Object{});
    j.build(raw);
    if (j.is_error()) {
      errmsg = static_cast<const char*>(j.data_);
      return Json(Object{});
    }
    return j;
  }

  static Json unmarshal(const std::string& raw, const char*& errmsg) {
    return unmarshal(raw.c_str(), errmsg);
  }

  bool is_null() const { return type_ & Type::Null; }

  bool is_bool() const { return type_ & Type::Bool; }

  bool is_integer() const { return type_ & Type::Integer; }

  bool is_float() const { return type_ & Type::Float; }

  bool is_object() const { return type_ & Type::Object; }

  bool is_array() const { return type_ & Type::Array; }

  bool is_string() const { return type_ & Type::String; }

 private:
  bool is_error() const { return type_ & Type::Error; }

  bool is_use_data() const {
    return type_ & (Type::String | Type::Object | Type::Array | Type::Error);
  }

  bool add_kv(std::string key, Json value);

  bool add_elem(Json value);

  void free() {
    if (is_string()) {
      delete static_cast<StringData*>(data_);
    } else if (is_object()) {
      delete static_cast<ObjectData*>(data_);
    } else if (is_array()) {
      delete static_cast<ArrayData*>(data_);
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
    return Json((size_t)Type::Error, (void*)errmsg);
  }

 private:
  size_t type_;
  union {
    bool b_;       // store bool value
    long long i_;  // store integer value
    double f_;     // store float value
    void* data_;   // the underlaying data of string, object, array
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
