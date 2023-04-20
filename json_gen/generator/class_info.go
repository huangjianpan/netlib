package generator

import (
	"strings"
)

const (
	AccessPublic    = "public"
	AccessPrivate   = "private"
	AccessProtected = "protected"
	Struct          = "struct"
	Class           = "class"
)

const (
	FieldTypePOD = iota
	FieldTypeString
	FieldTypeMap
	FieldTypeArray
	FieldTypeCustom
	FieldTypeRawPointer
)

// "type": {
// 	"desugaredQualType": "std::map<std::__cxx11::basic_string<char>, std::vector<unsigned long, std::allocator<unsigned long> >, std::less<std::__cxx11::basic_string<char> >, std::allocator<std::pair<const std::__cxx11::basic_string<char>, std::vector<unsigned long, std::allocator<unsigned long> > > > >",
// 	"qualType": "std::map<std::string, std::vector<size_t> >"
//   }

var PODTypeMap = map[string]struct{}{
	"bool":          {},
	"int":           {},
	"unsigned int":  {},
	"long":          {},
	"unsigned long": {},
	"long long":     {},
	"float":         {},
	"double":        {},
	"long double":   {},
}

var TypeMapping = map[string]string{
	"std::__cxx11::basic_string<char>": "std::string",
}

type (
	ClassInfo struct {
		ClassName string
		Location  *Location
		Range     *Range
		File      string   // 头文件
		Fields    []*Field // 字段
	}
	Field struct {
		ClassName string     // class name
		Access    string     // public/protected/private
		Attr      *Attribute // Attribute(...)
		Type      *FieldType // type
		Name      string     // field name
		Location  *Location  // the location of field name
	}
	FieldType struct {
		Name  string       // 全名，如std::map<std::string, XXX>
		Type  int64        // FieldTypeXXX
		Value []*FieldType // 模板参数列表
	}
)

func ParseFieldType(t string) *FieldType {
	var parse func(r []rune, begin, end int) (*FieldType, int)
	parse = func(r []rune, begin, end int) (*FieldType, int) {
		postProcess := func(f *FieldType) { // 后处理
			if _, ok := PODTypeMap[f.Name]; ok {
				f.Type = FieldTypePOD
				return
			}
			if _, ok := TypeMapping[f.Name]; ok {
				f.Name = TypeMapping[f.Name]
			}
			if f.Name == "std::string" {
				f.Type = FieldTypeString
			}
		}

		res := &FieldType{
			Type: FieldTypeCustom,
		}
		// 1. 先找到<，说明有模板列表；2. 先找到,或者>，说明没有模版列表
		left, right := -1, end
		for i := begin; i < end; i++ {
			if r[i] == ',' || r[i] == '>' {
				right = i
				break
			} else if r[i] == '<' {
				left = i
				break
			}
		}
		if left == -1 { // 没有模板参数
			res.Name = strings.TrimSpace(string(r[begin:right]))
		} else {
			right = left
			for i := left; i < end; {
				if r[i] == '<' || r[i] == ',' {
					value, next := parse(r, i+1, end)
					res.Value = append(res.Value, value)
					i = next
				} else if r[i] == '>' {
					right = i + 1
					break
				} else {
					i++
				}
			}
			prefix := strings.TrimSpace(string(r[begin:left]))
			switch prefix {
			case "std::vector":
				res.Name = prefix + "<" + res.Value[0].Name + ">"
				res.Type = FieldTypeArray
			case "std::map":
				res.Name = prefix + "<" + res.Value[0].Name + ", " + res.Value[1].Name + ">"
				res.Type = FieldTypeMap
			default:
				res.Name = prefix + "<" + res.Value[0].Name
				for i := 1; i < len(res.Value); i++ {
					res.Name += ", " + res.Value[i].Name
				}
				res.Name += ">"
			}
		}
		postProcess(res)
		return res, right
	}
	res, _ := parse([]rune(t), 0, len(t))
	return res
}
