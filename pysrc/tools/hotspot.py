import subprocess
import argparse


class HotspotManager:
    def __init__(self):
        pass

    def start(self, name='MyHotspot', password=None, authentication='wpa-psk', encryption='aes', channel=11, max_clients=10):
        if password is None:
            raise ValueError("Password is required when starting a hotspot")
        cmd = [
            'nmcli', 'dev', 'wifi', 'hotspot', 'ifname', 'wlan0', 'ssid', name, 'password', password
        ]
        self._run_command(cmd)
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.security', authentication])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.band', 'bg'])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.channel', str(channel)])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless.cloned-mac-address', 'stable'])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless.mac-address-randomization', 'no'])
        print(f"Hotspot {name} is now running with password {password}")

    def stop(self):
        self._run_command(['nmcli', 'connection', 'down', 'Hotspot'])
        print("Hotspot has been stopped")

    def status(self):
        status = self._run_command(['nmcli', 'dev', 'status'])
        if 'connected' in status:
            print("Hotspot is running")
            self._run_command(['nmcli', 'connection', 'show', 'Hotspot'])
        else:
            print("Hotspot is not running")

    def list(self):
        self._run_command(['nmcli', 'connection', 'show', '--active'])

    def set(self, name='MyHotspot', password=None, authentication='wpa-psk', encryption='aes', channel=11, max_clients=10):
        if password is None:
            raise ValueError(
                "Password is required when setting a hotspot profile")
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.ssid', name])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless-security.key-mgmt', authentication])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless-security.proto', 'rsn'])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless-security.group', encryption])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless-security.pairwise', encryption])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless-security.psk', password])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.band', 'bg'])
        self._run_command(['nmcli', 'connection', 'modify',
                          'Hotspot', '802-11-wireless.channel', str(channel)])
        self._run_command(['nmcli', 'connection', 'modify', 'Hotspot',
                          '802-11-wireless.mac-address-randomization', 'no'])
        print(f"Hotspot profile '{name}' has been updated")

    def _run_command(self, cmd):
        try:
            result = subprocess.run(
                cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            return result.stdout
        except subprocess.CalledProcessError as e:
            print(e.stderr)
            return e.stderr


def main():
    parser = argparse.ArgumentParser(description='Manage WiFi Hotspot')
    parser.add_argument('action', choices=[
                        'Start', 'Stop', 'Status', 'List', 'Set'], help='Action to perform')
    parser.add_argument('--name', default='MyHotspot', help='Hotspot name')
    parser.add_argument('--password', help='Hotspot password')
    parser.add_argument('--authentication', default='wpa-psk',
                        choices=['wpa-psk', 'wpa2'], help='Authentication type')
    parser.add_argument('--encryption', default='aes',
                        choices=['aes', 'tkip'], help='Encryption type')
    parser.add_argument('--channel', type=int,
                        default=11, help='Channel number')
    parser.add_argument('--max_clients', type=int, default=10,
                        help='Maximum number of clients')

    args = parser.parse_args()

    manager = HotspotManager()

    if args.action == 'Start':
        manager.start(args.name, args.password, args.authentication,
                      args.encryption, args.channel, args.max_clients)
    elif args.action == 'Stop':
        manager.stop()
    elif args.action == 'Status':
        manager.status()
    elif args.action == 'List':
        manager.list()
    elif args.action == 'Set':
        manager.set(args.name, args.password, args.authentication,
                    args.encryption, args.channel, args.max_clients)


if __name__ == "__main__":
    main()
