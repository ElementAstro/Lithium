import React from "react";
import {
  IndexRouteObject,
  NonIndexRouteObject,
  useRoutes,
} from "react-router-dom";

import {
  Motherboard,
  Server,
  Box,
  Gear,
  InfoCircle,
} from "react-bootstrap-icons";

import Welcome from "../pages/home/animation";
import Home from "../pages/home";
import Dashboard from "../pages/dashboard";
import ServerSearch from "../pages/server";
import DeviceConnection from "../pages/connect";
import DeviceControlPanelPage from "../pages/device";
import ConfigManager from "../pages/settings";
import ModuleManager from "../pages/module";
import Help from "../pages/about/help";
import AboutPage from "../pages/about";

import GlobalParameterSettingPage from "../pages/config";
import Connect from "../pages/connection/index";
import Helper from "../pages/helper";
import ErrorPage from "../pages/error";

interface IndexRouteObjectPlus extends IndexRouteObject {
  title?: string;
  icon?: React.ReactNode;
}

interface NonIndexRouteObjectPlus extends NonIndexRouteObject {
  title?: string;
  icon?: React.ReactNode;
}

type RouteObjectPlus = IndexRouteObjectPlus | NonIndexRouteObjectPlus;

export const routesConfig: RouteObjectPlus[] = [
  {
    path: "/",
    element: <Welcome />,
    title: "首页",
    icon: <InfoCircle fill="#333" width={28} height={28}></InfoCircle>,
  },
  {
    path: "/home",
    element: <Home />,
    title: "欢迎",
    icon: <InfoCircle fill="#333" width={28} height={28}></InfoCircle>,
  },
  {
    path: "/dashboard",
    element: <Dashboard />,
    title: "Dashboard",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/server",
    element: <ServerSearch />,
    title: "服务器连接",
    icon: <Server fill="#333" width={28} height={28}></Server>,
  },
  {
    path: "/device_connect",
    element: <DeviceConnection />,
    title: "设备连接",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
  {
    path: "/device",
    element: <DeviceControlPanelPage />,
    title: "设备控制",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
  {
    path: "/connect",
    element: <Connect />,
    title: "连接",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
  {
    path: "/settings",
    element: <ConfigManager />,
    title: "配置管理",
    icon: <Gear fill="#333" width={28} height={28}></Gear>,
  },
  {
    path : "/config",
    element: <GlobalParameterSettingPage />,
    title: "全局参数设置",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/module",
    element: <ModuleManager />,
    title: "模块管理",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/help",
    element: <Help />,
    title: "帮助",
    icon: <InfoCircle fill="#333" width={28} height={28}></InfoCircle>,
  },
  {
    path: "/helper",
    element: <Helper />,
    title: "辅助工具",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/about",
    element: <AboutPage />,
    title: "关于",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/error",
    element: <ErrorPage />,
    title: "错误",
    icon: <InfoCircle fill="#333" width={28} height={28}></InfoCircle>,
  }
];

const Route = () => {
  const element = useRoutes(routesConfig);
  return element;
};

export default Route;
