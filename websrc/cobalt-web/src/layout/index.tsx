import { useEffect } from "react";
import Header from "./header";
import Navigation from "./navigation";
import { useNavigate } from "react-router-dom";
import "./style.less";
import React from "react";

type Props = {
  children: JSX.Element | JSX.Element[] | string | string[];
};

export default ({ children }: Props) => {
  const navigate = useNavigate();

  useEffect(() => {
    navigate("/");
  }, []);

  return (
    <div className="app-main">
      <Header></Header>
      <Navigation />
      <main>{children}</main>
    </div>
  );
};
