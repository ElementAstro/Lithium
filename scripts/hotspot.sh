#!/bin/bash

# hotspot.sh
#
# 用于在 Linux 上管理无线热点的 Shell 脚本
# 使用 nmcli (NetworkManager 命令行接口)
#
# 使用方法：
#   ./hotspot.sh {Start|Stop|Status|List|Set} [Name] [Password] [Authentication] [Encryption] [Channel] [MaxClients]
#
# 参数：
#   - Action: 必需。操作类型。可选值为 Start、Stop、Status、List、Set。
#   - Name: 热点名称。默认值为 "MyHotspot"。
#   - Password: 热点的密码。在 Start 和 Set 操作中是必需的。
#   - Authentication: 认证类型。可选值为 wpa-psk 或 wpa2。默认值为 wpa-psk。
#   - Encryption: 加密类型。可选值为 aes 或 tkip。默认值为 aes。
#   - Channel: 热点使用的频道号。默认值为 11。
#   - MaxClients: 最大客户端数量。默认值为 10。
#
# 使用示例：
#   启动热点：
#     ./hotspot.sh Start MyHotspot MyPassword
#
#   停止热点：
#     ./hotspot.sh Stop
#
#   查看热点状态：
#     ./hotspot.sh Status
#
#   列出所有热点配置：
#     ./hotspot.sh List
#
#   设置热点配置文件：
#     ./hotspot.sh Set MyHotspot MyPassword wpa-psk aes 11 10

# 参数解析
ACTION=$1
NAME=${2:-MyHotspot}
PASSWORD=$3
AUTHENTICATION=${4:-wpa-psk}
ENCRYPTION=${5:-aes}
CHANNEL=${6:-11}
MAX_CLIENTS=${7:-10}

# 启动热点
function start_hotspot {
    if [ -z "$PASSWORD" ]; then
        echo "Password is required when starting a hotspot"
        exit 1
    fi

    # 创建热点连接
    nmcli dev wifi hotspot ifname wlan0 ssid "$NAME" password "$PASSWORD"

    # 设置其他参数
    nmcli connection modify Hotspot 802-11-wireless.security "$AUTHENTICATION"
    nmcli connection modify Hotspot 802-11-wireless.band bg
    nmcli connection modify Hotspot 802-11-wireless.channel "$CHANNEL"
    nmcli connection modify Hotspot 802-11-wireless.cloned-mac-address stable
    nmcli connection modify Hotspot 802-11-wireless.mac-address-randomization no

    echo "Hotspot $NAME is now running with password $PASSWORD"
}

# 停止热点
function stop_hotspot {
    nmcli connection down Hotspot
    echo "Hotspot has been stopped"
}

# 查看热点状态
function status_hotspot {
    status=$(nmcli dev status | grep wlan0)
    if echo "$status" | grep -q "connected"; then
        echo "Hotspot is running"
        nmcli connection show Hotspot
    else
        echo "Hotspot is not running"
    fi
}

# 列出所有热点配置
function list_hotspot {
    nmcli connection show --active | grep wifi
}

# 设置热点配置文件
function set_hotspot {
    if [ -z "$PASSWORD" ]; then
        echo "Password is required when setting a hotspot profile"
        exit 1
    fi

    # 创建或修改热点连接
    nmcli connection modify Hotspot 802-11-wireless.ssid "$NAME"
    nmcli connection modify Hotspot 802-11-wireless-security.key-mgmt "$AUTHENTICATION"
    nmcli connection modify Hotspot 802-11-wireless-security.proto rsn
    nmcli connection modify Hotspot 802-11-wireless-security.group "$ENCRYPTION"
    nmcli connection modify Hotspot 802-11-wireless-security.pairwise "$ENCRYPTION"
    nmcli connection modify Hotspot 802-11-wireless-security.psk "$PASSWORD"
    nmcli connection modify Hotspot 802-11-wireless.band bg
    nmcli connection modify Hotspot 802-11-wireless.channel "$CHANNEL"
    nmcli connection modify Hotspot 802-11-wireless.mac-address-randomization no

    echo "Hotspot profile '$NAME' has been updated"
}

# 主程序入口
case $ACTION in
    Start)
        start_hotspot
        ;;
    Stop)
        stop_hotspot
        ;;
    Status)
        status_hotspot
        ;;
    List)
        list_hotspot
        ;;
    Set)
        set_hotspot
        ;;
    *)
        echo "Usage: $0 {Start|Stop|Status|List|Set} [Name] [Password] [Authentication] [Encryption] [Channel] [MaxClients]"
        ;;
esac
