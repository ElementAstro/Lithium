# Example 1: Start a WiFi hotspot
# This example starts a WiFi hotspot with the specified name and password.
$ python hotspot.py start --name MyHotspot --password mypassword

# Example 2: Start a WiFi hotspot with custom settings
# This example starts a WiFi hotspot with custom authentication, encryption, channel, max clients, band, and hidden SSID.
$ python hotspot.py start --name MyHotspot --password mypassword --authentication wpa2 --encryption aes --channel 6 --max-clients 5 --band ac --hidden

# Example 3: Stop the WiFi hotspot
# This example stops the currently running WiFi hotspot.
$ python hotspot.py stop

# Example 4: Check the status of the WiFi hotspot
# This example checks and displays the current status of the WiFi hotspot.
$ python hotspot.py status

# Example 5: List connected clients
# This example lists all clients currently connected to the WiFi hotspot.
$ python hotspot.py list-clients

# Example 6: Save a hotspot profile
# This example saves the current hotspot configuration as a profile with the specified name.
$ python hotspot.py save-profile --profile MyProfile --name MyHotspot --authentication wpa2 --encryption aes --channel 6 --max-clients 5 --band ac --hidden

# Example 7: Load a hotspot profile and start hotspot
# This example loads the specified hotspot profile and starts the WiFi hotspot with the loaded configuration.
$ python hotspot.py load-profile --profile MyProfile