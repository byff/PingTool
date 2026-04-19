package excel

import (
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"

	"github.com/xuri/excelize/v2"
)

type PingResult struct {
	Num      int
	Hostname string
	IP       string
	Success  int
	Fail     int
	FailPct  string
	Total    int
	RTT      string
	RTTMax   string
	RTTMin   string
	RTTAvg   string
	Status   int
}

type ExcelService struct{}

func NewExcelService() *ExcelService {
	return &ExcelService{}
}

func (s *ExcelService) ImportTxt(path string) ([]string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	lines := strings.Split(string(data), "\n")
	result := make([]string, 0)

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line != "" {
			result = append(result, line)
		}
	}

	return result, nil
}

func (s *ExcelService) ImportExcel(path string) ([]string, int, error) {
	f, err := excelize.OpenFile(path)
	if err != nil {
		return nil, -1, err
	}
	defer f.Close()

	sheets := f.GetSheetList()
	if len(sheets) == 0 {
		return nil, -1, fmt.Errorf("no sheets found")
	}

	rows, err := f.GetRows(sheets[0])
	if err != nil {
		return nil, -1, err
	}

	if len(rows) == 0 {
		return nil, -1, fmt.Errorf("empty sheet")
	}

	header := rows[0]
	ipCol := findIPColumn(header)
	if ipCol == -1 {
		for rowIdx, row := range rows {
			if rowIdx == 0 {
				continue
			}
			for colIdx, cell := range row {
				if isIP(cell) || isDomain(cell) {
					ipCol = colIdx
					break
				}
			}
			if ipCol != -1 {
				break
			}
		}
	}

	if ipCol == -1 {
		return nil, -1, fmt.Errorf("no IP column found")
	}

	result := make([]string, 0)
	for rowIdx, row := range rows {
		if rowIdx == 0 {
			continue
		}
		if ipCol < len(row) {
			cell := strings.TrimSpace(row[ipCol])
			if cell != "" {
				result = append(result, cell)
			}
		}
	}

	return result, ipCol, nil
}

func (s *ExcelService) ExportExcel(results []PingResult, path string) error {
	f := excelize.NewFile()
	sheet := "Sheet1"

	for idx, row := range results {
		rowNum := idx + 1
		f.SetCellValue(sheet, fmt.Sprintf("A%d", rowNum), row.Num)
		f.SetCellValue(sheet, fmt.Sprintf("B%d", rowNum), row.Hostname)
		f.SetCellValue(sheet, fmt.Sprintf("C%d", rowNum), row.IP)
		f.SetCellValue(sheet, fmt.Sprintf("D%d", rowNum), row.Success)
		f.SetCellValue(sheet, fmt.Sprintf("E%d", rowNum), row.Fail)
		f.SetCellValue(sheet, fmt.Sprintf("F%d", rowNum), row.FailPct)
		f.SetCellValue(sheet, fmt.Sprintf("G%d", rowNum), row.Total)
		f.SetCellValue(sheet, fmt.Sprintf("H%d", rowNum), row.RTT)
		f.SetCellValue(sheet, fmt.Sprintf("I%d", rowNum), row.RTTMax)
		f.SetCellValue(sheet, fmt.Sprintf("J%d", rowNum), row.RTTMin)
		f.SetCellValue(sheet, fmt.Sprintf("K%d", rowNum), row.RTTAvg)
	}

	headers := []string{"#", "主机名", "IP", "成功", "失败", "失败率", "总计", "延迟", "最大", "最小", "平均"}
	for colIdx, header := range headers {
		colLetter, _ := excelize.ColumnNumberToName(colIdx + 1)
		f.SetCellValue(sheet, fmt.Sprintf("%s1", colLetter), header)
	}

	return f.SaveAs(path)
}

func (s *ExcelService) InsertToSource(results []PingResult, sourcePath string, colIndex int) error {
	f, err := excelize.OpenFile(sourcePath)
	if err != nil {
		return err
	}
	defer f.Close()

	sheets := f.GetSheetList()
	sheet := sheets[0]

	startRow := 2
	for rowIdx := startRow; rowIdx <= len(results)+1; rowIdx++ {
		resultIdx := rowIdx - startRow
		if resultIdx >= len(results) {
			break
		}

		r := results[resultIdx]
		colLetter, _ := excelize.ColumnNumberToName(colIndex + 1)
		f.SetCellValue(sheet, fmt.Sprintf("%s%d", colLetter, rowIdx), fmt.Sprintf("%s|%s|%s|%s", r.Success, r.Fail, r.FailPct, r.RTT))
	}

	return f.SaveFile(sourcePath)
}

func findIPColumn(headers []string) int {
	ipKeywords := []string{"ip", "address", "addr", "主机", "ip地址"}
	for colIdx, header := range headers {
		headerLower := strings.ToLower(header)
		for _, keyword := range ipKeywords {
			if strings.Contains(headerLower, keyword) {
				return colIdx
			}
		}
	}
	return -1
}

func isIP(s string) bool {
	parts := strings.Split(s, ".")
	if len(parts) != 4 {
		return false
	}
	for _, part := range parts {
		if n, err := strconv.Atoi(part); err != nil || n < 0 || n > 255 {
			return false
		}
	}
	return true
}

func isDomain(s string) bool {
	domainRegex := regexp.MustCompile(`^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$`)
	return domainRegex.MatchString(s)
}
