package main

import (
	"encoding/json"
	"fmt"
	"json_gen/generator"
	"os"
)

// sudo apt install clang-10 --install-suggests

type Input struct {
	Clang             string   `json:"clang"`
	IncludeDirectorys []string `json:"include_directory"`
	HeaderFiles       []string `json:"cxx_files"`
	// OutputFile        string   `json:"output_file"`
}

func GetInput(config string) (*Input, error) {
	bs, err := os.ReadFile(config)
	if err != nil {
		return nil, err
	}
	input := &Input{}
	if err := json.Unmarshal(bs, input); err != nil {
		return nil, err
	}
	return input, nil
}

func main() {
	if len(os.Args) < 2 {
		fmt.Printf(`usage: ./json_gen <config.json>
example:
    create a json file named "config.json", contents:
    {
        "clang": "clang-10",
        "cxx_files": ["model.h"],
        "include_directory": ["./json"]
    }
explain:
   clang -- command name of clang.
   cxx_files -- files that need to generate code.
   include_directory -- 'cxx_file' depend on the header file diectory. 
execute: 
   './json_gen config.json' to generate code.
`)
		return
	}
	input, err := GetInput(os.Args[1])
	if err != nil {
		os.Stderr.Write([]byte(fmt.Sprintf("started failed, error: %v", err)))
		return
	}
	p := generator.NewGenerator(input.Clang, input.HeaderFiles, input.IncludeDirectorys)
	if err := p.Generate(); err != nil {
		fmt.Println(err)
	}
}
