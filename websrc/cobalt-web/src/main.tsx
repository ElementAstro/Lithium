import React from "react";
import ReactDOM from "react-dom/client";

import Route from "./routes/root.tsx";
import { Router } from "react-router";

import "./index.css";
import App from "./App";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
