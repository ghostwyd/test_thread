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
var g_total int

func get_out(c chan string, i int) {
	k := rand.Int() % 10
	time.Sleep((time.Duration)(k) * time.Millisecond)
	str := fmt.Sprintf("i am talking to %d.  timestamp: %d", i, k)
	c <- str
}

func write_file(content []byte) int {
	file_name := fmt.Sprintf("%d.jpg", g_total)
	g_total += 1
	fmt.Printf("file name:%s", file_name)
	err := ioutil.WriteFile(file_name, content, 0777)
	if err != nil {
		return -1
	}
	return 0
}
func get_root_name(url string) string {
	ip_reg := regexp.MustCompile("^((https?://)?([\\d]{1,3}\\.){3}[\\d]{1,3})[?/#].*$")
	str := ip_reg.FindStringSubmatch(url)
	if str == nil {
		domain_reg := regexp.MustCompile("^((https?://)?([\\d\\w-]{1,62}\\.){2,4}([\\d\\w-]{1,62}))([/?#].*)?$")
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
		fmt.Printf("%s is already searched!\n", url)
		ch <- 1
		return
	}

	rsp, err := http.Get(url)
	root := get_root_name(url)
	if root == "" {
		ch <- 1
		fmt.Printf("get root fail for %s\n", url)
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
	/*for key, value := range rsp.Header {
		fmt.Printf("key:%s, value:%s\n", key, value)
	}*/
	if value, ok := (rsp.Header)["Content-Type"]; ok {
		if value[0] == "image/jpeg" {
			write_file(body)
		}

	}

	//fmt.Printf("%s", string(body))
	g_set[url] = 0
	reg_list := [2]string{"a href=\"([^ ]*)\"", "src=\"([^ ]*)\""}
	var url_reg *regexp.Regexp
	sub_chan := make(chan int)
	sub_count := 0

	blank_reg := regexp.MustCompile("[\\s]")
	for _, reg := range reg_list {
		url_reg = regexp.MustCompile(reg)
		str_array := url_reg.FindAllStringSubmatch(string(body), -1)

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
			tmp = blank_reg.ReplaceAllLiteralString(tmp, "")
			_, ok := g_set[tmp]
			if !ok {
				//g_set[tmp] = 0
				//fmt.Printf("depth:%d, %s\n", depth, tmp)
				sub_count++
				go crawl(tmp, depth-1, sub_chan)
			}
		}
	}

	for i := 0; i < sub_count; i++ {
		<-sub_chan
	}
	ch <- 1
	return
}

func main() {
	g_total = 0
	depth := 4
	g_set = make(map[string]int)
	var root_chan chan int
	root_chan = make(chan int)
	crawl("http://www.myddm.com/thread-451704-1-1.html", depth, root_chan)
	root := get_root_name("thread-445955-3-1.html")
	fmt.Println("domain name ", root)
	<-root_chan
	/*str := "http://.myddm.com/attachment.php?aid=MTQwNDA1N3w4MDU0ZTgyMHwxMzk0NjQwMTg3fGQwNWV5ZWdaWVlzQ1lydDFhLzhwdFp3WDl3OUV4NW1vS2VxMGcrUVU4QzdtaXpr&amp;nothumb=yes"
	blank_reg := regexp.MustCompile("\r\n|\n")
	fmt.Println(blank_reg.ReplaceAllLiteralString(str, ""))*/
	return
}
