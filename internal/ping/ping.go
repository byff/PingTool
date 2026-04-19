package ping

import (
	"fmt"
	"net"
	"sync"
	"sync/atomic"
	"time"

	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv4"
)

type PingResult struct {
	Num      int    `json:"num"`
	Hostname string `json:"hostname"`
	IP       string `json:"ip"`
	Success  int    `json:"success"`
	Fail     int    `json:"fail"`
	FailPct  string `json:"fail_pct"`
	Total    int    `json:"total"`
	RTT      string `json:"rtt"`
	RTTMax   string `json:"rtt_max"`
	RTTMin   string `json:"rtt_min"`
	RTTAvg   string `json:"rtt_avg"`
	Status   int    `json:"status"`
}

type target struct {
	hostname string
	ip       string
}

type PingService struct {
	results      []PingResult
	resultsMutex sync.RWMutex
	running      atomic.Bool
	stopChan     chan struct{}
	callback     func([]PingResult)
}

func NewPingService() *PingService {
	return &PingService{
		results:  make([]PingResult, 0),
		stopChan: make(chan struct{}),
	}
}

func (s *PingService) SetResultCallback(callback func([]PingResult)) {
	s.callback = callback
}

func (s *PingService) Start(targets []string, timeout, packetSize, interval, concurrent int) error {
	if s.running.Load() {
		return fmt.Errorf("ping is already running")
	}

	s.results = make([]PingResult, 0)
	s.running.Store(true)
	s.stopChan = make(chan struct{})

	go s.run(targets, timeout, packetSize, interval, concurrent)
	return nil
}

func (s *PingService) Stop() {
	if s.running.Load() {
		s.running.Store(false)
		close(s.stopChan)
	}
}

func (s *PingService) GetResults() []PingResult {
	s.resultsMutex.RLock()
	defer s.resultsMutex.RUnlock()
	return s.results
}

func (s *PingService) run(targets []string, timeout, packetSize, interval, concurrent int) {
	taskChan := make(chan target, len(targets))
	resultChan := make(chan *PingResult, len(targets))
	var wg sync.WaitGroup

	for i := 0; i < concurrent && i < len(targets); i++ {
		wg.Add(1)
		go s.worker(&wg, taskChan, resultChan, timeout, packetSize, interval)
	}

	go func() {
		for _, t := range targets {
			select {
			case <-s.stopChan:
				return
			case taskChan <- target{hostname: t, ip: t}:
			}
		}
		close(taskChan)
	}()

	go func() {
		wg.Wait()
		close(resultChan)
	}()

	for result := range resultChan {
		s.resultsMutex.Lock()
		result.Num = len(s.results) + 1
		s.results = append(s.results, *result)
		resultsCopy := make([]PingResult, len(s.results))
		copy(resultsCopy, s.results)
		s.resultsMutex.Unlock()

		if s.callback != nil {
			s.callback(resultsCopy)
		}
	}

	s.running.Store(false)
}

func (s *PingService) worker(wg *sync.WaitGroup, tasks <-chan target, results chan<- *PingResult, timeout, packetSize, interval int) {
	defer wg.Done()

	for t := range tasks {
		select {
		case <-s.stopChan:
			return
		default:
			result := s.ping(t, timeout, packetSize, interval)
			results <- result
		}
	}
}

func (s *PingService) ping(t target, timeout, packetSize, interval int) *PingResult {
	result := &PingResult{
		Hostname: t.hostname,
		IP:       t.hostname,
		RTT:      "-",
		RTTMax:   "-",
		RTTMin:   "-",
		RTTAvg:   "-",
		FailPct:  "-",
		Status:   -1,
	}

	ips, err := net.LookupHost(t.hostname)
	if err != nil || len(ips) == 0 {
		if net.ParseIP(t.hostname) == nil {
			result.Fail = 1
			result.Status = 2
			return result
		}
		ips = []string{t.hostname}
	}

	result.IP = ips[0]
	result.Hostname = t.hostname

	conn, err := icmp.ListenPacket("ip4:icmp", "")
	if err != nil {
		result.Fail = 1
		result.Status = 2
		return result
	}
	defer conn.Close()

	seq := 1
	totalSuccess := 0
	totalFail := 0
	minRTT := int64(-1)
	maxRTT := int64(0)
	totalRTT := int64(0)

	for i := 0; i < 10 && s.running.Load(); i++ {
		start := time.Now()
		conn.SetWriteDeadline(start.Add(time.Duration(timeout) * time.Millisecond))
		conn.SetReadDeadline(start.Add(time.Duration(timeout) * time.Millisecond))

		msg := &icmp.Message{
			Type: ipv4.ICMPTypeEcho, Code: 0,
			Body: &icmp.Echo{
				ID:   seq,
				Seq:  seq,
				Data: make([]byte, packetSize),
			},
		}

		data, err := msg.Marshal(nil)
		if err != nil {
			totalFail++
			seq++
			continue
		}

		dst, _ := net.ResolveIPAddr("ip4", result.IP)
		_, err = conn.WriteTo(data, dst)
		if err != nil {
			totalFail++
			seq++
			time.Sleep(time.Duration(interval) * time.Millisecond)
			continue
		}

		reply := make([]byte, 1024)
		_, _, err := conn.ReadFrom(reply)
		if err != nil {
			totalFail++
			seq++
			time.Sleep(time.Duration(interval) * time.Millisecond)
			continue
		}

		rtt := time.Since(start).Milliseconds()
		totalSuccess++
		totalRTT += rtt

		if minRTT == -1 || rtt < minRTT {
			minRTT = rtt
		}
		if rtt > maxRTT {
			maxRTT = rtt
		}

		if reply[0] == 0 {
			ttl := int(reply[8])
			_ = ttl
		}

		result.RTT = fmt.Sprintf("%dms", rtt)
		seq++
		time.Sleep(time.Duration(interval) * time.Millisecond)
	}

	result.Success = totalSuccess
	result.Fail = totalFail
	result.Total = totalSuccess + totalFail

	if result.Total > 0 {
		if totalSuccess > 0 {
			result.RTTAvg = fmt.Sprintf("%dms", totalRTT/int64(totalSuccess))
			if minRTT != -1 {
				result.RTTMin = fmt.Sprintf("%dms", minRTT)
			}
			result.RTTMax = fmt.Sprintf("%dms", maxRTT)
		}
		failPct := float64(totalFail) / float64(result.Total) * 100
		result.FailPct = fmt.Sprintf("%.0f%%", failPct)

		if failPct == 0 {
			result.Status = 0
		} else if failPct < 50 {
			result.Status = 1
		} else {
			result.Status = 2
		}
	}

	return result
}
