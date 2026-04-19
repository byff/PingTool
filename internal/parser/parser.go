package parser

import (
	"fmt"
	"net"
	"regexp"
	"strconv"
	"strings"
)

func ParseTargets(input string) ([]string, error) {
	lines := strings.Split(input, "\n")
	result := make([]string, 0)
	seen := make(map[string]bool)

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		targets, err := parseLine(line)
		if err != nil {
			continue
		}

		for _, t := range targets {
			if !seen[t] {
				seen[t] = true
				result = append(result, t)
			}
		}
	}

	return result, nil
}

func parseLine(line string) ([]string, error) {
	line = strings.TrimSpace(line)
	if line == "" {
		return nil, nil
	}

	if isCIDR(line) {
		return parseCIDR(line)
	}

	if isIPRange(line) {
		return parseIPRange(line)
	}

	if isIP(line) || isDomain(line) {
		return []string{line}, nil
	}

	return nil, fmt.Errorf("invalid format: %s", line)
}

func isCIDR(s string) bool {
	_, _, err := net.ParseCIDR(s)
	return err == nil
}

func isIPRange(s string) bool {
	parts := strings.Split(s, "-")
	if len(parts) != 2 {
		return false
	}
	ip := net.ParseIP(strings.TrimSpace(parts[0]))
	if ip == nil {
		return false
	}
	rest := strings.TrimSpace(parts[1])
	if net.ParseIP(rest) != nil {
		return true
	}
	_, err := strconv.Atoi(rest)
	return err == nil
}

func isIP(s string) bool {
	return net.ParseIP(s) != nil
}

func isDomain(s string) bool {
	domainRegex := regexp.MustCompile(`^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$`)
	return domainRegex.MatchString(s)
}

func parseCIDR(cidr string) ([]string, error) {
	_, ipNet, err := net.ParseCIDR(cidr)
	if err != nil {
		return nil, err
	}

	result := make([]string, 0)
	for ip := ipNet.IP.Mask(ipNet.Mask); ipNet.Contains(ip); incIP(ip) {
		result = append(result, ip.String())
		if len(result) >= 65536 {
			break
		}
	}

	return result, nil
}

func parseIPRange(s string) ([]string, error) {
	parts := strings.Split(s, "-")
	start := strings.TrimSpace(parts[0])
	end := strings.TrimSpace(parts[1])

	startIP := net.ParseIP(start)
	if startIP == nil {
		return nil, fmt.Errorf("invalid start IP: %s", start)
	}

	var endIP net.IP
	if net.ParseIP(end) != nil {
		endIP = net.ParseIP(end)
	} else {
		endNum, err := strconv.Atoi(end)
		if err != nil {
			return nil, fmt.Errorf("invalid end: %s", end)
		}
		endIP = make(net.IP, len(startIP))
		copy(endIP, startIP)
		endIP[3] = byte(endNum)
	}

	result := make([]string, 0)
	for ip := startIP; !ip.Equal(endIP); incIP(ip) {
		result = append(result, ip.String())
		if len(result) >= 65536 {
			break
		}
	}
	result = append(result, endIP.String())

	return result, nil
}

func incIP(ip net.IP) {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
}
