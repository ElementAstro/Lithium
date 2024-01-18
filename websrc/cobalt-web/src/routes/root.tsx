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

import Dashboard from "../pages/dashboard";
import ServerSearch from "../pages/connection";
import DeviceConnection from "../pages/connect";
import ConfigManager from "../pages/settings";
import ModuleManager from "../pages/module";
import Help from "../pages/help";
import AboutPage from "../pages/about";

import Connect from "../pages/connection/index";
import Helper from "../pages/helper";

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
    path: "/dashboard",
    element: <Dashboard />,
    title: "Dashboard",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
  {
    path: "/",
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
];

const Route = () => {
  const element = useRoutes(routesConfig);
  return element;
};

export default Route;
