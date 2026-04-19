package main

import (
	"context"
	"pingtool/internal/config"
	"pingtool/internal/excel"
	"pingtool/internal/parser"
	"pingtool/internal/ping"
)

type App struct {
	ctx              context.Context
	pingService      *ping.PingService
	configService    *config.ConfigService
	excelService     *excel.ExcelService
}

func NewApp() *App {
	return &App{
		pingService:   ping.NewPingService(),
		configService: config.NewConfigService(),
		excelService:  excel.NewExcelService(),
	}
}

func (a *App) startup(ctx context.Context) {
	a.ctx = ctx
}

func (a *App) SetPingResultCallback(callback func([]ping.PingResult)) {
	a.pingService.SetResultCallback(callback)
}

func (a *App) StartPing(targets []string, timeout, packetSize, interval, concurrent int) error {
	return a.pingService.Start(targets, timeout, packetSize, interval, concurrent)
}

func (a *App) StopPing() {
	a.pingService.Stop()
}

func (a *App) GetResults() []ping.PingResult {
	return a.pingService.GetResults()
}

func (a *App) ParseTargets(input string) ([]string, error) {
	return parser.ParseTargets(input)
}

func (a *App) ImportTxt(path string) ([]string, error) {
	return a.excelService.ImportTxt(path)
}

type ImportResult struct {
	Ips      []string `json:"ips"`
	ColIndex int      `json:"colIndex"`
}

func (a *App) ImportExcel(path string) (*ImportResult, error) {
	ips, colIndex, err := a.excelService.ImportExcel(path)
	if err != nil {
		return nil, err
	}
	return &ImportResult{Ips: ips, ColIndex: colIndex}, nil
}

func (a *App) ExportExcel(results []ping.PingResult, path string) error {
	return a.excelService.ExportExcel(results, path)
}

func (a *App) InsertToSource(results []ping.PingResult, sourcePath string, colIndex int) error {
	return a.excelService.InsertToSource(results, sourcePath, colIndex)
}

func (a *App) LoadConfig() (*config.Config, error) {
	return a.configService.Load()
}

func (a *App) SaveConfig(cfg *config.Config) error {
	return a.configService.Save(cfg)
}
