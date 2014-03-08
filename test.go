package main

import (
	"fmt"
	"math/rand"
	"time"
    "net/http"
)

var g_set map[string]int

func get_out(c chan string, i int) {
    k := rand.Int() % 10
    time.Sleep((time.Duration)(k) * time.Millisecond)
	str := fmt.Sprintf("i am talking to %d.  timestamp: %d", i, k)
	c <- str
}

func get_url(url string) {
    rsp, err :=  http.Get("http://www.myddm.com")
    if  ! err {
        fmt.Printf("get %s fail!\n", url);
    }
    return
}

func main() {
	var c chan string
	c = make(chan string)
    g_set = make(map[string]int)
    g_set["test"] = 0
	for i := 0; i < 10; i++ {
		go get_out(c, i)
	}
	for i := 0; i < 10; i++ {
		fmt.Println(<-c)
	}
}
