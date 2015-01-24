package main

import (
	"fmt"
	"github.com/willglynn/go-opencl/cl"
)

func main() {
    platforms, err := cl.GetPlatforms()
    if err != nil {
        fmt.Printf("Failed to get platforms: %+v", err)
    }
	fmt.Println("Got",len(platforms),"Platforms");
}
