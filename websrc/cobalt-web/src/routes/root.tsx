import React from "react";
import {
  IndexRouteObject,
  NonIndexRouteObject,
  useRoutes,
} from "react-router-dom";

import { Motherboard } from "react-bootstrap-icons";

import DeviceConnection from "../pages/connect";

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
    element: <DeviceConnection />,
    title: "设备连接",
    icon: <Motherboard fill="#333" width={28} height={28}></Motherboard>,
  },
];

const Route = () => {
  const element = useRoutes(routesConfig);
  return element;
};

export default Route;
