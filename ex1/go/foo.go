package main

import (
	. "fmt"
	"runtime"
)

const MAX_ITERATION = 1000000

func inc(c chan int, quit chan string) {

	for d := 0; d < MAX_ITERATION; d++ {
		c <- (<-c) + 1
	}
	quit <- "quit"

}

func dec(c chan int, quit chan string) {

	for d := 0; d < MAX_ITERATION; d++ {
		c <- (<-c) - 1
	}
	quit <- "quit"

}

func main() {

	runtime.GOMAXPROCS(2)

	c := make(chan int, 1)
	quit := make(chan string, 2)

	c <- 0
	quit <- "start"
	quit <- "start"

	go inc(c, quit)
	go dec(c, quit)

	for {
		if <-quit == "quit" {
			if <-quit == "quit" {

				Println("Value:", <-c)
				return
			}
		}
	}

}
