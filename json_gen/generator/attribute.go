package generator

const (
	AttributeKeyWord = "Attribute"
)

type (
	Attribute struct {
		Message  string // format: json:"xxx,omitempty"
		JsonAttr *JsonAttr
	}

	JsonAttr struct {
		Name      string
		OmitEmpty bool
	}
)
