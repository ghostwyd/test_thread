package main

import (
	"fmt"
	"time"
)

func get_out(c chan string, i int) {
	str := fmt.Sprintf("i am talking to %d\n", i)
	time.Sleep(())
	c <- str
}

func main() {
	var c chan string
	c = make(chan string)
	for i := 0; i < 10; i++ {
		go get_out(c, i)
	}
	for i := 0; i < 10; i++ {
		fmt.Println(<-c)
	}
}
