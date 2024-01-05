import React from "react";
import {
  IndexRouteObject,
  NonIndexRouteObject,
  useRoutes,
} from "react-router-dom";

import { Motherboard,Server, Box } from "react-bootstrap-icons";

import ServerSearch from "../pages/connection";
import DeviceConnection from "../pages/connect";
import ConfigManager from "../pages/config";
import ModuleManager from "../pages/module";

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
    element: <ServerSearch />,
    title: "服务器连接",
    icon: <Server fill="#333" width={28} height={28}></Server>,
  },
  {
    path: "/connect",
    element: <DeviceConnection />,
    title: "设备连接",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
  {
    path: "/config",
    element: <ConfigManager />,
    title: "配置管理",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
  {
    path: "/module",
    element: <ModuleManager />,
    title: "模块管理",
    icon: <Box fill="#333" width={28} height={28}></Box>,
  },
];

const Route = () => {
  const element = useRoutes(routesConfig);
  return element;
};

export default Route;
