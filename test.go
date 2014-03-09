package main

import (
	"fmt"
	"io/ioutil"
	"math/rand"
	"net/http"
	"regexp"
	"time"
)

var g_set map[string]int

func get_out(c chan string, i int) {
	k := rand.Int() % 10
	time.Sleep((time.Duration)(k) * time.Millisecond)
	str := fmt.Sprintf("i am talking to %d.  timestamp: %d", i, k)
	c <- str
}

func get_root_name(url string) string {
	ip_reg := regexp.MustCompile("^((https?://)?([\\d]{1,3}\\.){3}[\\d]{1,3})[?/#].*$")
	str := ip_reg.FindStringSubmatch(url)
	if str == nil {
		domain_reg := regexp.MustCompile("^((https?://)?([\\d\\w-]{1,62}\\.){2,3}([\\d\\w-]{1,62}))([/?#].*)?$")
		str = domain_reg.FindStringSubmatch(url)
	}
	if str == nil {
		//		fmt.Printf("%s is not a legal url!\n", url)
		return ""
	}
	if str != nil {
		return str[1]
	}
	return ""

}
func crawl(url string, depth int, ch chan int) {

	if depth < 1 {
		ch <- 1
		return
	}
	if _, ok := g_set[url]; ok {
		//fmt.Printf("%s is already searched!\n", url)
		ch <- 1
		return
	}

	rsp, err := http.Get(url)
	root := get_root_name(url)
	if root == "" {
		ch <- 1
		return
	}

	if err != nil {
		fmt.Printf("get %s fail!\n", url)
		ch <- 1
		return
	}
	fmt.Printf("return status:%s, proto:%s\n", rsp.Status, rsp.Proto)
	/*for key, value := range rsp.Header {
		fmt.Printf("key=%s, value=%s\n ", key, value)
	}*/
	defer rsp.Body.Close()
	body, err := ioutil.ReadAll(rsp.Body)
	if err != nil {
		fmt.Printf("read response body error!\n")
		ch <- 1
		return
	}
	url_reg := regexp.MustCompile("a href=\"([^ ]*)\"")
	str_array := url_reg.FindAllStringSubmatch(string(body), -1)
	sub_count := 0
	var sub_chan chan int
	sub_chan = make(chan int)
	for _, href := range str_array {
		if len(href) == 0 {
			continue
		}
		if href[0] == "" || href[1] == "" {
			continue
		}
		tmp := href[1]
		if get_root_name(tmp) == "" {
			tmp = fmt.Sprint(root, "/", tmp)
		}
		_, ok := g_set[tmp]
		if !ok {
			g_set[tmp] = 0
			fmt.Println(tmp)
			sub_count++
			go crawl(tmp, depth-1, sub_chan)
		}
	}
	for i := 0; i < sub_count; i++ {
		<-sub_chan
	}
	ch <- 1

}

func main() {
	depth := 100
	g_set = make(map[string]int)
	var root_chan chan int
	root_chan = make(chan int)
	//go crawl("http://www.myddm.com/thread-439222-1-2.html", depth, root_chan)
	go crawl("http://www.sina.com", depth, root_chan)
	root := get_root_name("thread-445955-3-1.html")
	fmt.Println("domain name ", root)
	/*for key, _ := range g_set {
		fmt.Println(key)
	}*/
	<-root_chan
	return
}



