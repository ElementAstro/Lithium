import React, { useEffect } from "react";
import Navigation from "./navigation";
import { useNavigate } from "react-router-dom";
import "./style.less";

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
      {<Navigation />}
      <main className="app-body">{children}</main>
    </div>
  );
};
