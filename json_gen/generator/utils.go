package generator

import (
	"bufio"
	"os"
)

// TernaryOperator 三元运算符
func TernaryOperator(condition bool, trueRet string, falseRet string) string {
	if condition {
		return trueRet
	} else {
		return falseRet
	}
}

func ReadFile(fileName string) ([]string, error) {
	file, err := os.Open(fileName)
	if err != nil {
		return nil, err
	}
	fileScanner := bufio.NewScanner(file)
	fileScanner.Split(bufio.ScanLines)
	var line []string
	for fileScanner.Scan() {
		line = append(line, fileScanner.Text())
	}
	file.Close()
	return line, nil
}

func SkipBlank(s string, begin int) int {
	for begin < len(s) && s[begin] == ' ' {
		begin++
	}
	return begin
}

func SkipBlankOpposite(s string, begin int) int {
	for begin >= 0 && s[begin] == ' ' {
		begin--
	}
	return begin
}
