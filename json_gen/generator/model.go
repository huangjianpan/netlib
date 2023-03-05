package generator

const (
	TranslationUnitDecl = "TranslationUnitDecl"
	FieldDecl           = "FieldDecl"
	CXXRecordDecl       = "CXXRecordDecl"
	NamespaceDecl       = "NamespaceDecl"
	AccessSpecDecl      = "AccessSpecDecl"
)

type (
	IncludeFrom struct {
		File string `json:"file"`
	}

	Location struct {
		Offset      int64        `json:"offset"`
		File        string       `json:"file"`
		Line        int64        `json:"line"`
		Col         int64        `json:"col"`
		TokLen      int64        `json:"tokLen"`
		IncludeFrom *IncludeFrom `json:"includedFrom,omitempty"`
	}

	Range struct {
		Begin Location `json:"begin"`
		End   Location `json:"end"`
	}

	Type struct {
		QualType          string `json:"qualType"`
		DesugaredQualType string `json:"desugaredQualType"` // 完整类型
	}

	AST struct {
		Kind    string    `json:"kind"`
		Loc     *Location `json:"loc,omitempty"`
		Range   *Range    `json:"range,omitempty"`
		Name    string    `json:"name"` // the name of namespace, class, field
		Inner   []*AST    `json:"inner,omitempty"`
		TagUsed string    `json:"tagUsed"`          // Kind = CXXRecordDecl, TagUsed = class/struct
		Type    *Type     `json:"type,omitempty"`   // Kind = FieldDecl
		Access  string    `json:"access,omitempty"` // Kind = AccessSpecDecl, Access = public/protected/private
	}
)
