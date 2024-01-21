import React, { useEffect } from "react";
import { AppProviders } from "./AppProvider";
import { HashRouter as Router } from "react-router-dom";
import Body from "./layout/index";
import Route from "./routes/root";

import WS_LISTENER_COMP from "./pages/home/init_ws_listener";

import "bootstrap/dist/css/bootstrap.min.css";

function Main() {
  // useEffect(()=>{
  //   getResouceList().then(res=>{
  //     console.log(res);
  //   });
  // },[])

  return (
    <Router>
      <Body>
        <Route></Route>
      </Body>
      <WS_LISTENER_COMP />
    </Router>
  );
}

export default () => (
  <AppProviders>
    <Main></Main>
  </AppProviders>
);
