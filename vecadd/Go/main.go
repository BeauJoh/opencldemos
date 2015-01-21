package main

import (
	"fmt"
	"github.com/cassianokc/cl"
)

func main() {
	fmt.Println("Getting platforms")
	platforms, err := GetPlatforms()
	if err != nil {
		t.Fatalf("Failed to get platforms: %+v", err)
	}
	for i, p := range platforms {
		t.Logf("Platform %d:", i)
		t.Logf("  Name: %s", p.Name())
		t.Logf("  Vendor: %s", p.Vendor())
		t.Logf("  Profile: %s", p.Profile())
		t.Logf("  Version: %s", p.Version())
		t.Logf("  Extensions: %s", p.Extensions())
	}
	fmt.Println("Got Platforms")
}
