import { useEffect } from 'react';
import { AppProviders } from './AppProvider';
import { HashRouter as Router } from 'react-router-dom';
import Body from './layout/index';
import Route from "./routes/root";
import { getResouceList } from './services/api';
import WS_LISTENER_COMP from './pages/home/init_ws_listener';

import "bootstrap/dist/css/bootstrap.min.css";
import React from 'react';

function Main() {

  // useEffect(()=>{
  //   getResouceList().then(res=>{
  //     console.log(res);
  //   });
  // },[])


  return (
    <Router >
      <Body >
        <Route></Route>
      </Body>
      <WS_LISTENER_COMP />
    </Router>)
}


export default () => (
  <AppProviders>
    <Main></Main>
  </AppProviders>
)
