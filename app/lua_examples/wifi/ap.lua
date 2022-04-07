-- Device work as AP

require "wmwifi"
wmwifi.init()
wmwifi.ap("your_ssid", "your_password")