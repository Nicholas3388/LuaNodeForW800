-- Device works as Station and connects to specific SSID

require "wmwifi"
wmwifi.init()
wmwifi.sta("target_ssid", "password")