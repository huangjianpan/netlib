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
	return NewParseType(t).Parse()
}

type ParseType struct {
	AllName  string
	Brackets map[int]int
}

func NewParseType(t string) *ParseType {
	return &ParseType{
		AllName:  t,
		Brackets: make(map[int]int),
	}
}

func (p *ParseType) Parse() *FieldType {
	p.RemoveBlank()
	p.TypeMapping()
	p.GenerateBracketMapping()
	return p.parseEntry(p.AllName, 0, len(p.AllName))
}

func (p *ParseType) RemoveBlank() {
	names := strings.Fields(p.AllName)
	p.AllName = ""
	for _, n := range names {
		p.AllName += n
	}
}

func (p *ParseType) TypeMapping() {
	for k, v := range TypeMapping {
		p.AllName = strings.ReplaceAll(p.AllName, k, v)
	}
}

func (p *ParseType) GenerateBracketMapping() {
	left := make([]int, 0)
	for i := 0; i < len(p.AllName); i++ {
		if p.AllName[i] == '<' {
			left = append(left, i)
		} else if p.AllName[i] == '>' {
			p.Brackets[left[len(left)-1]] = i
			left = left[:len(left)-1]
		}
	}
}

func (p *ParseType) parseEntry(t string, begin int, end int) *FieldType {
	left := strings.IndexRune(t[begin:end], '<')
	if left == -1 { // 没有模板列表
		typeName := t[begin:end]
		if _, ok := PODTypeMap[typeName]; ok {
			return &FieldType{
				Name: typeName,
				Type: FieldTypePOD,
			}
		}
		if typeName == "std::string" {
			return &FieldType{
				Name: typeName,
				Type: FieldTypeString,
			}
		}
		return &FieldType{
			Name: typeName,
			Type: FieldTypeCustom,
		}
	}
	// 存在模板列表
	left += begin
	right := p.Brackets[left]
	prefix := t[begin:left]
	if prefix == "std::vector" {
		values := p.parseList(t, left+1, right)
		return &FieldType{
			Name:  "std::vector<" + values[0].Name + ">",
			Type:  FieldTypeArray,
			Value: values,
		}
	} else if prefix == "std::map" {
		values := p.parseList(t, left+1, right)
		return &FieldType{
			Name:  "std::map<" + values[0].Name + ", " + values[1].Name + ">",
			Type:  FieldTypeMap,
			Value: values,
		}
	} else {
		values := p.parseList(t, left+1, right)
		name := prefix + "<"
		for i := 0; i < len(values)-1; i++ {
			name += values[i].Name + ", "
		}
		name += values[len(values)-1].Name + ">"
		return &FieldType{
			Name:  name,
			Type:  FieldTypeCustom,
			Value: values,
		}
	}
}

func (p *ParseType) parseList(t string, begin int, end int) []*FieldType {
	pre := begin
	var values []*FieldType
	for i := begin; i < end; i++ {
		if t[i] == ',' {
			values = append(values, p.parseEntry(t, pre, i))
			pre = i + 1
		} else if t[i] == '<' {
			j := p.Brackets[i]
			values = append(values, p.parseEntry(t, pre, j+1))
			pre = j + 1
			i = j
		}
	}
	values = append(values, p.parseEntry(t, pre, end))
	return values
}
