package config

import (
	"os"
	"path/filepath"

	"github.com/pelletier/go-toml/v2"
)

type Config struct {
	Timeout       int    `json:"timeout"`
	PacketSize    int    `json:"packet_size"`
	Interval      int    `json:"interval"`
	MaxConcurrent int    `json:"max_concurrent"`
	CIDREnabled   bool   `json:"cidr_enabled"`
	RememberIPs   bool   `json:"remember_ips"`
	IPList        string `json:"ip_list"`
}

type ConfigService struct {
	configPath string
}

func NewConfigService() *ConfigService {
	home, _ := os.UserHomeDir()
	configDir := filepath.Join(home, ".config", "pingtool")
	os.MkdirAll(configDir, 0755)

	return &ConfigService{
		configPath: filepath.Join(configDir, "config.toml"),
	}
}

func (s *ConfigService) Load() (*Config, error) {
	data, err := os.ReadFile(s.configPath)
	if err != nil {
		return &Config{
			Timeout:       1000,
			PacketSize:    64,
			Interval:      1000,
			MaxConcurrent: 100,
			CIDREnabled:   true,
			RememberIPs:   true,
		}, nil
	}

	var cfg Config
	if err := toml.Unmarshal(data, &cfg); err != nil {
		return nil, err
	}

	return &cfg, nil
}

func (s *ConfigService) Save(cfg *Config) error {
	data, err := toml.Marshal(cfg)
	if err != nil {
		return err
	}

	return os.WriteFile(s.configPath, data, 0644)
}
