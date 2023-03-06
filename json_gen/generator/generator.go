package generator

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// gen ast json file:
// clang++ -Xclang -ast-dump=json -fsyntax-only -Iinclude -x c++ [header file] > [output ast json file]

// 1. RunCmdAndLoadAST: 运行clang命令生成json语法树并过滤读取
// 2. GenClassInfos: 利用json ast找出所有类信息
// 3. SearchAttribute: 打开头文件，查找Attribute信息
// 4. FilterAndCheckFields

type Generator struct {
	Clang            string                // clang命令名
	CXXHeaderFiles   []string              // the c++ header file
	IncludeDirectory []string              // c++头文件依赖的头文件所在目录
	FileAST          map[string]*AST       // 存储clang处理的ast结构
	ClassInfos       map[string]*ClassInfo // 类名 -> *ClassInfo
	FileClass        map[string][]string   // 文件 -> 类名
	CurrentAccess    string                // 用于生成类信息
	HadGenCodeType   map[string]struct{}   // 已经生成代码的类型
	Code             []string              // 生成的代码
	ErrMsg           error
}

func NewGenerator(clang string, headerFiles []string, includeDirectory []string) *Generator {
	return &Generator{
		Clang:            clang,
		CXXHeaderFiles:   headerFiles,
		IncludeDirectory: includeDirectory,
		FileAST:          make(map[string]*AST),
		ClassInfos:       make(map[string]*ClassInfo),
		FileClass:        make(map[string][]string),
		HadGenCodeType:   make(map[string]struct{}),
		Code:             make([]string, 0),
	}
}

func (p *Generator) Generate() error {
	return p.RunCmdAndLoadAST().GenClassInfos().SearchAttribute().FilterAndCheckFields().
		GenerateCode().PrintCodes().ErrMsg
}

// RunCmdAndLoadAST 执行clang命令生成语法树json
func (p *Generator) RunCmdAndLoadAST() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	var include string
	for _, i := range p.IncludeDirectory {
		include += fmt.Sprintf("-I %s ", i)
	}
	for _, file := range p.CXXHeaderFiles {
		clangCmd := fmt.Sprintf(`%s -Xclang -ast-dump=json -fsyntax-only %s -x c++ "%s"`, p.Clang, include, file)
		cmd := exec.Command("/bin/sh", "-c", clangCmd)
		raw, _ := cmd.Output() // 忽略错误，std::map<int, std::vector<int>>会报错
		data := &AST{}
		if err := json.Unmarshal(raw, data); err != nil {
			p.ErrMsg = fmt.Errorf("unmarshal raw ast json error, file:%s, err:%v", file, err)
			break
		}
		p.filter(data) // 过滤掉不必要的信息
		p.FileAST[file] = data
	}
	return p
}

// GenClassInfos 生成类信息
func (p *Generator) GenClassInfos() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	for file, ast := range p.FileAST {
		p.genClassInfos(file, ast, []string{})
	}
	return p
}

func (p *Generator) PrintClassInfos() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	collectFieldsByAccess := func(fields []*Field, access string) []*Field {
		var ret []*Field
		for _, f := range fields {
			if f.Access == access {
				ret = append(ret, f)
			}
		}
		return ret
	}
	for name, info := range p.ClassInfos {
		if len(info.Fields) == 0 {
			continue
		}
		fmt.Printf("struct %s { // line:[%d, %d]\n", name, info.Location.Line, info.Range.End.Line)
		for _, access := range []string{AccessPublic, AccessPrivate, AccessProtected} {
			ret := collectFieldsByAccess(info.Fields, access)
			if len(ret) > 0 {
				fmt.Printf(" %s:\n", access)
				for _, f := range ret {
					if f.Attr != nil {
						fmt.Printf("  %s %s `%s`\n", f.Type.Name, f.Name, f.Attr.Message)
					} else {
						fmt.Printf("  %s %s\n", f.Type.Name, f.Name)
					}
				}
			}
		}
		fmt.Println("}")
	}
	return p
}

// SearchAttribute 搜索Attribute关键字，补充Attribute信息
func (p *Generator) SearchAttribute() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	parseJsonAttr := func(attr string) (*JsonAttr, bool) {
		idx := strings.Index(attr, "json")
		if idx == -1 || attr[idx+4] != ':' {
			return nil, false
		}
		idx = SkipBlank(attr, idx+5)
		idx2 := SkipBlankOpposite(attr, len(attr)-1) + 1
		if idx2 == -1 || idx2-idx < 1 {
			return nil, false
		}
		return &JsonAttr{
			Name: attr[idx:idx2],
		}, true
	}

	for file, classNames := range p.FileClass {
		lines, err := ReadFile(file)
		if err != nil {
			p.ErrMsg = err
			break
		}
		for _, className := range classNames {
			info := p.ClassInfos[className]
			for i := info.Location.Line - 1; i <= info.Range.End.Line-1; i++ {
				// 找到Attribute信息
				attr, err := findAttributeInfo(lines[i])
				if err != nil {
					p.ErrMsg = fmt.Errorf("file:%s, line:%d, error:%v", file, i+1, err)
					return p
				}
				if len(attr) == 0 { // 该行没有Attribute
					continue
				}
				// 拼接Attribute到对应字段名之间的所有文本
				endLine := i
				for j := i; j <= info.Range.End.Line-1; j++ {
					begin := 0
					if j == i {
						begin = strings.Index(lines[i], attr) + len(attr) + 2
					}
					if idx := strings.IndexRune(lines[j][begin:], ';'); idx != -1 {
						endLine = j
						break
					}
				}
				var allLine string
				for j := i; j <= endLine; j++ {
					allLine += lines[j] + " "
				}
				// 找到字段名
				slice := strings.Fields(allLine)
				if len(slice) < 3 {
					p.ErrMsg = fmt.Errorf("search attribute error, file:%s, line:%d, class:%s", file, i+1, className)
					return p
				}
				last := len(slice) - 1
				fieldName := TernaryOperator(slice[len(slice)-1] == ";", slice[last-1], slice[last][:len(slice[last])-1])
				// 补充字段的Attribute信息
				for _, f := range info.Fields {
					if f.Name == fieldName {
						if jsonAttr, ok := parseJsonAttr(attr); ok {
							f.Attr = &Attribute{
								Message:  attr,
								JsonAttr: jsonAttr,
							}
						} else {
							p.ErrMsg = fmt.Errorf("attribute format error, attribute:%s, file:%s, line:%d", attr, file, i+1)
							return p
						}
					}
				}
			}
		}
	}
	return p
}

func (p *Generator) FilterAndCheckFields() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	// 去除没有Attribute的字段和类
	p.filterClassInfoWithoutAttribute()
	for _, info := range p.ClassInfos {
		for _, f := range info.Fields {
			if !p.checkAccess(f) {
				classInfo := p.ClassInfos[f.ClassName]
				os.Stderr.Write([]byte(fmt.Sprintf("file:%s, line:%d, %s::%s is %s\n",
					classInfo.File, f.Location.Line, f.ClassName, f.Name, f.Access)))
				p.ErrMsg = errors.New("check field failed")
			}
			if !p.checkFieldType(f, f.Type) {
				classInfo := p.ClassInfos[f.ClassName]
				os.Stderr.Write([]byte(fmt.Sprintf("file:%s, line:%d, %s::%s is not a supported type, type: %s\n",
					classInfo.File, f.Location.Line, f.ClassName, f.Name, f.Type.Name)))
				p.ErrMsg = errors.New("check field failed")
			}
		}
	}
	return p
}

func (p *Generator) GenerateCode() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	for _, info := range p.ClassInfos {
		p.generateUnmarshalCode(&FieldType{
			Name: info.ClassName,
			Type: FieldTypeCustom,
		})
	}
	for k := range p.HadGenCodeType {
		delete(p.HadGenCodeType, k)
	}
	for _, info := range p.ClassInfos {
		p.generateMarshalCode(&FieldType{
			Name: info.ClassName,
			Type: FieldTypeCustom,
		})
	}
	return p
}

func (p *Generator) generateMarshalCode(f *FieldType) {
	switch f.Type {
	case FieldTypeMap:
		p.generateMarshalCodeMap(f)
	case FieldTypeArray:
		p.generateMarshalCodeArray(f)
	case FieldTypeCustom:
		p.generateMarshalCodeCustom(f)
	}
}

func (p *Generator) generateMarshalCodeMap(f *FieldType) {
	getCode := func(t *FieldType) string {
		switch t.Type {
		case FieldTypePOD:
			return "kv.second"
		case FieldTypeString:
			return "std::move(kv.second)"
		default:
			return "convert(std::move(kv.second))"
		}
	}
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		p.generateMarshalCode(f.Value[1])
		p.Code = append(p.Code, fmt.Sprintf(
			"inline json::Json convert(%s&& model) {\n"+
				"  json::Json j(json::Json::Object{});\n"+
				"  for (auto& kv : model) {\n"+
				"    j.add(kv.first, %s);\n"+
				"  }\n"+
				"  return j;\n"+
				"}", f.Name, getCode(f.Value[1])))
	}
}

func (p *Generator) generateMarshalCodeArray(f *FieldType) {
	getCode := func(t *FieldType) string {
		switch t.Type {
		case FieldTypePOD:
			return "elem"
		case FieldTypeString:
			return "std::move(elem)"
		default:
			return "convert(std::move(elem))"
		}
	}
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		p.generateMarshalCode(f.Value[0])
		p.Code = append(p.Code, fmt.Sprintf(
			"inline json::Json convert(%s&& model) {\n"+
				"  json::Json j(json::Json::Array{});\n"+
				"  for (auto& elem : model) {\n"+
				"    j.add(%s);\n"+
				"  }\n"+
				"  return j;\n"+
				"}", f.Name, getCode(f.Value[0])))
	}
}

func (p *Generator) generateMarshalCodeCustom(f *FieldType) {
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		code := fmt.Sprintf("inline json::Json convert(%s&& model) {\n"+
			"  json::Json j(json::Json::Object{});\n", f.Name)
		classInfo, exist := p.ClassInfos[f.Name]
		if !exist {
			p.ErrMsg = fmt.Errorf("generate code failed, type %s not exist", f.Name)
			return
		}
		for _, f := range classInfo.Fields {
			p.generateMarshalCode(f.Type)
			switch f.Type.Type {
			case FieldTypePOD:
				code += fmt.Sprintf("  j.add(\"%s\", model.%s);\n", f.Attr.JsonAttr.Name, f.Name)
			case FieldTypeString:
				code += fmt.Sprintf("  j.add(\"%s\", std::move(model.%s));\n", f.Attr.JsonAttr.Name, f.Name)
			default:
				code += fmt.Sprintf("  j.add(\"%s\", convert(std::move(model.%s)));\n", f.Attr.JsonAttr.Name, f.Name)
			}
		}
		code += "  return j;\n}"
		p.Code = append(p.Code, code)

		p.Code = append(p.Code, fmt.Sprintf(
			"inline std::string marshal(const %s& model) {\n"+
				"  return convert(%s(model)).marshal();\n"+
				"}", f.Name, f.Name))

		p.Code = append(p.Code, fmt.Sprintf(
			"inline std::string marshal(%s&& model) {\n"+
				"  return convert(std::move(model)).marshal();\n"+
				"}", f.Name))
	}
}

func (p *Generator) generateUnmarshalCode(f *FieldType) {
	switch f.Type {
	case FieldTypePOD, FieldTypeString:
		p.generateUnmarshalCodePODAndString(f)
	case FieldTypeMap:
		p.generateUnmarshalCodeMap(f)
	case FieldTypeArray:
		p.generateUnmarshalCodeArray(f)
	case FieldTypeCustom:
		p.generateUnmarshalCodeCustom(f)
	}
}

func (p *Generator) generateUnmarshalCodePODAndString(f *FieldType) {
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		p.Code = append(p.Code, fmt.Sprintf("inline void unmarshal(json::Json& j, %s& ret) {\n  j.move_to(ret);\n}", f.Name))
	}
}

func (p *Generator) generateUnmarshalCodeMap(f *FieldType) {
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		p.generateUnmarshalCode(f.Value[1])
		p.Code = append(p.Code, fmt.Sprintf("inline void unmarshal(json::Json& j, %s& ret) {\n"+
			"  j.move_to(ret, static_cast<void(*)(Json&, %s&)>(unmarshal));\n"+
			"}", f.Name, f.Value[1].Name))
	}
}

func (p *Generator) generateUnmarshalCodeArray(f *FieldType) {
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		p.generateUnmarshalCode(f.Value[0])
		p.Code = append(p.Code, fmt.Sprintf("inline void unmarshal(json::Json& j, %s& ret) {\n"+
			"  j.move_to(ret, static_cast<void(*)(Json&, %s&)>(unmarshal));\n"+
			"}", f.Name, f.Value[0].Name))
	}
}

func (p *Generator) generateUnmarshalCodeCustom(f *FieldType) {
	if _, ok := p.HadGenCodeType[f.Name]; !ok {
		p.HadGenCodeType[f.Name] = struct{}{}
		code := fmt.Sprintf("inline void unmarshal(json::Json& j, %s& ret) {\n", f.Name)
		classInfo, exist := p.ClassInfos[f.Name]
		if !exist {
			p.ErrMsg = fmt.Errorf("generate code failed, type %s not exist", f.Name)
			return
		}
		for _, f := range classInfo.Fields {
			p.generateUnmarshalCode(f.Type)
			code += fmt.Sprintf("  unmarshal(j[\"%s\"], ret.%s);\n", f.Attr.JsonAttr.Name, f.Name)
		}
		code += "}"
		p.Code = append(p.Code, code)

		p.Code = append(p.Code, fmt.Sprintf("inline const char* unmarshal(const char* raw, %s& ret) {\n"+
			"  const char* errmsg = nullptr;\n"+
			"  json::Json j = json::Json::unmarshal(raw, errmsg);\n"+
			"  if (errmsg != nullptr) {\n"+
			"    return errmsg;\n"+
			"  }\n"+
			"  unmarshal(j, ret);\n"+
			"  return nullptr;\n}", f.Name))

		p.Code = append(p.Code, fmt.Sprintf("inline const char* unmarshal(const std::string& raw, %s& ret) {\n"+
			"  return unmarshal(raw.c_str(), ret);\n}", f.Name))
	}
}

func (p *Generator) PrintCodes() *Generator {
	if p.ErrMsg != nil {
		return p
	}
	var s string
	s += "#pragma once\n\n"
	for _, file := range p.CXXHeaderFiles {
		s += fmt.Sprintf("#include \"%s\"\n", file)
	}
	s += fmt.Sprintf("#include \"%s\"\n", "json.h")
	fmt.Println(s)
	fmt.Printf("namespace json {\n\n")
	for _, code := range p.Code {
		fmt.Println(code)
		fmt.Println("")
	}
	fmt.Println("} // namespace json")
	return p
}

func (p *Generator) filterClassInfoWithoutAttribute() {
	for _, info := range p.ClassInfos {
		var filtered []*Field
		for _, f := range info.Fields {
			if f.Attr != nil {
				filtered = append(filtered, f)
			}
		}
		if len(filtered) == 0 {
			delete(p.ClassInfos, info.ClassName)
		}
		info.Fields = filtered
	}
}

func (p *Generator) checkAccess(f *Field) bool {
	return f.Access == AccessPublic
}

func (p *Generator) checkFieldType(f *Field, t *FieldType) bool {
	ok := false
	switch t.Type {
	case FieldTypePOD, FieldTypeString:
		ok = true
	case FieldTypeArray:
		ok = p.checkFieldType(f, t.Value[0])
	case FieldTypeMap:
		if t.Value[0].Type != FieldTypeString {
			ok = false
		} else {
			ok = p.checkFieldType(f, t.Value[1])
		}
	case FieldTypeRawPointer:
		ok = false
	case FieldTypeCustom:
		if _, exist := p.ClassInfos[t.Name]; exist {
			ok = true
		}
	default:
	}
	return ok
}

func findAttributeInfo(line string) (string, error) {
	idx := strings.Index(line, AttributeKeyWord)
	if idx == -1 {
		return "", nil
	}
	idx += len(AttributeKeyWord)
	if line[idx] != '(' || line[idx+1] != '"' {
		return "", fmt.Errorf("mismatch left (\"")
	}
	begin := idx + 2
	end := strings.IndexRune(line[begin:], '"') + begin
	if end == -1 || len(line) <= end+1 || line[end+1] != ')' {
		return "", fmt.Errorf("mismatch right \")")
	}
	if end-begin < 1 {
		return "", errors.New("attribute is empty")
	}
	return line[begin:end], nil
}

func (p *Generator) genClassInfos(file string, ast *AST, namespaces []string) {
	genClassName := func() string {
		var name string
		for i := 0; i < len(namespaces)-1; i++ {
			name += namespaces[i] + "::"
		}
		return name + namespaces[len(namespaces)-1]
	}
	switch ast.Kind {
	case NamespaceDecl:
		namespaces = append(namespaces, ast.Name)
		defer func() {
			namespaces = namespaces[:len(namespaces)-1]
		}()
	case CXXRecordDecl:
		namespaces = append(namespaces, ast.Name)
		defer func() {
			namespaces = namespaces[:len(namespaces)-1]
		}()
		info := &ClassInfo{
			ClassName: genClassName(),
			Location:  ast.Loc,
			Range:     ast.Range,
			File:      file,
		}
		p.CurrentAccess = TernaryOperator(ast.TagUsed == Struct, AccessPublic, AccessPrivate)
		p.ClassInfos[info.ClassName] = info
		p.FileClass[file] = append(p.FileClass[file], info.ClassName)
	case AccessSpecDecl:
		p.CurrentAccess = ast.Access
	case FieldDecl:
		className := genClassName()
		p.ClassInfos[className].Fields = append(p.ClassInfos[className].Fields, &Field{
			ClassName: className,
			Access:    p.CurrentAccess,
			Name:      ast.Name,
			Type:      ParseFieldType(TernaryOperator(len(ast.Type.DesugaredQualType) == 0, ast.Type.QualType, ast.Type.DesugaredQualType)),
			Location:  ast.Loc,
		})
	}
	for _, elem := range ast.Inner {
		p.genClassInfos(file, elem, namespaces)
	}
}

func (p *Generator) filter(ast *AST) *AST {
	if ast == nil {
		return nil
	}
	isNeedKind := func(kind string) bool {
		switch kind {
		case TranslationUnitDecl, FieldDecl, CXXRecordDecl, NamespaceDecl, AccessSpecDecl:
			return true
		}
		return false
	}
	var inner []*AST
	for _, v := range ast.Inner {
		if !isNeedKind(v.Kind) {
			continue
		}
		if ast.Kind != TranslationUnitDecl && len(ast.Name) == 0 {
			continue
		}
		if ast.Kind == NamespaceDecl && ast.Name == "std" { // TODO：只过滤掉第一个namespace为std的ast
			continue
		}
		if ast.Kind == CXXRecordDecl {
			if ast.Loc != nil && ast.Loc.IncludeFrom != nil {
				continue
			}
			if ast.Range != nil {
				if ast.Range.Begin.IncludeFrom != nil || ast.Range.End.IncludeFrom != nil {
					continue
				}
			}
		}
		if v = p.filter(v); v != nil {
			inner = append(inner, v)
		}
	}
	if ast.Kind != FieldDecl && ast.Kind != AccessSpecDecl && len(inner) == 0 {
		return nil
	}
	ast.Inner = inner
	return ast
}
