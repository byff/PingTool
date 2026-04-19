package main

import (
	"embed"
	"log"

	"github.com/wailsapp/wails/v2"
	"github.com/wailsapp/wails/v2/pkg/options"
	"github.com/wailsapp/wails/v2/pkg/options/assetserver"
	"github.com/wailsapp/wails/v2/pkg/options/windows"
)

//go:embed all:frontend/dist
var assets embed.FS

func main() {
	app := NewApp()

	err := wails.Run(
		options.Windows{
			Title:                   "PingTool - 多目标Ping工具",
			Width:                   1200,
			Height:                  800,
			MinWidth:                900,
			MinHeight:               600,
			WindowStartState:        options.Normal,
			EnableInspector:         false,
			DisableWindowIcon:       false,
			WebviewInstallPath:      "",
			BackgroundColour:        &options.RGBA{R: 24, G: 26, B: 32, A: 255},
			Callback:                nil,
			Menu:                    nil,
			OnBeforeClose:           nil,
			OnClose:                 nil,
			OnStartup:               nil,
			OnDomReady:              nil,
			OnBeforeRender:          nil,
			OnRender:                nil,
			OnMenuOpenChromeDevTools: nil,
		},
		assetserver.Options{
			Assets: assets,
		},
	)

	if err != nil {
		log.Fatal(err)
	}
}
